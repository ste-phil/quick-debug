import { writable, type Writable } from "svelte/store";
import { ReadWritable } from "./Utils";

const dbName = 'RecordingDatabase';
const storeName = 'Recordings';

export class Record {
  constructor(public dataFlow: string, public data: number) {
  }
}

export class RecordingStats {
  constructor(public entries: Map<string, number>, public total: number) {
  }
}

export class Recording {
  public recordingData: Array<Record> = new Array<Record>();

  constructor(public name: string) {
  }

  // Returns a map with the number of entries for each dataFlow
  gatherStats(): RecordingStats {
    const stats = new Map<string, number>();
    this.recordingData.forEach((record) => {
      if (!stats.has(record.dataFlow)) {
        stats.set(record.dataFlow, 0);
      }
      stats.set(record.dataFlow, stats.get(record.dataFlow)! + 1);
    });
    return new RecordingStats(stats, this.recordingData.length);
  }

  // Returns a CSV string with the recording data
  convertToCSV(dataFlowsToExport: string[]): string {
    // Convert recording to CSV format
    const groupedEntries = this.recordingData.reduce(
      (acc: any, record: Record) => {
        if (!acc[record.dataFlow]) {
          acc[record.dataFlow] = new Array<number>();
        }
        acc[record.dataFlow].push(record.data);
        return acc;
      },
      {}
    ) as { [key: string]: number[] };

    // Remove all dataFlows that are not in dataFlowsToExport
    if (dataFlowsToExport.length !== 0) { // If dataFlowsToExport is empty, export all dataFlows
      for (const [name] of Object.entries(groupedEntries)) {
        if (!dataFlowsToExport.includes(name)) {
          delete groupedEntries[name];
        }
      }
    }

    //Find longest array in groupedEntries
    const longestArray = Object.values(groupedEntries).reduce(
      (acc, value) => (value.length > acc ? value.length : acc),
      0
    );

    //Write all the elements in the groupedEntries to a CSV string
    // One entry is a column in the CSV
    let csvContent = "";

    // Write the header
    csvContent += "Time,";
    for (const [name] of Object.entries(groupedEntries)) {
      csvContent += `${name},`;
    }
    csvContent += "\n";

    for (let i = 0; i < longestArray; i++) {
      csvContent += i + ",";
      for (const [name, values] of Object.entries(groupedEntries)) {
        csvContent += `${values[i] ?? ""},`;
      }
      csvContent += "\n";
    }

    return csvContent;
  }
}


export class RecordingManager {
  public isRecording: Writable<boolean>;
  private activeRecording: Recording | null = null;
  private finishedRecordings: Writable<Recording[]>

  get recordings(): Writable<Recording[]> {
    return this.finishedRecordings;
  }

  constructor() {
    this.isRecording = writable(false);
    this.finishedRecordings = writable([]);

    this.getAllRecordings()
      .then((recordings) => {
        this.finishedRecordings.set(recordings);
      })
      .catch((error) => {
        console.error('Recordings loading failed', error);
      });
  }

  startRecording(name: string) {
    console.log("Recording started: ", name);
    this.endRecording();
    this.activeRecording = new Recording(name);
    this.isRecording.set(true);
  }

  endRecording() {
    if (this.activeRecording) {
      console.log("Recording ended");

      this.finishedRecordings.update(x => [...x, this.activeRecording!]);

      this.storeRecording(this.activeRecording)
        .then(() => {
          console.log('Recording data saved');
        })
        .catch((error) => {
          console.error('Error saving recording data', error);
        });

      this.isRecording.set(false);
      this.activeRecording = null;
    }
  }

  record(dataFlow: string, data: number) {
    if (this.activeRecording) {
      this.activeRecording.recordingData.push(new Record(dataFlow, data));
    }
  }

  async deleteRecording(recordingName: string) {
    const db = await this.openDatabase();
    const transaction = db.transaction(storeName, 'readwrite');
    const store = transaction.objectStore(storeName);

    this.recordings.update((recordings) => recordings.filter((recording) => recording.name !== recordingName));
    store.delete(recordingName);

    return new Promise<void>((resolve, reject) => {
      transaction.oncomplete = () => {
        resolve();
      };

      transaction.onerror = () => {
        reject(transaction.error);
      };
    });
  }

  async deleteAllRecordings() {
    const db = await this.openDatabase();
    const transaction = db.transaction(storeName, 'readwrite');
    const store = transaction.objectStore(storeName);

    store.clear();  // Clear all records from the object store

    return new Promise<void>((resolve, reject) => {
      transaction.oncomplete = () => {
        this.finishedRecordings.set([]);  // Clear the local store as well
        resolve();
      };

      transaction.onerror = () => {
        reject(transaction.error);
      };
    });
  }

  private async storeRecording(recording: Recording) {
    const db = await this.openDatabase();
    const transaction = db.transaction(storeName, 'readwrite');
    const store = transaction.objectStore(storeName);

    let uniqueName = await this.getUniqueRecordingName(recording.name, store);
    recording.name = uniqueName;
    this.recordings.update((recordings) => recordings); // force reload of ui

    store.put(recording, recording.name); // Store using the unique name

    return new Promise<void>((resolve, reject) => {
      transaction.oncomplete = () => {
        resolve();
      };

      transaction.onerror = () => {
        reject(transaction.error);
      };
    });
  }

  private async getAllRecordings(): Promise<Recording[]> {
    const db = await this.openDatabase();
    const transaction = db.transaction(storeName, 'readonly');
    const store = transaction.objectStore(storeName);

    return new Promise<Recording[]>((resolve, reject) => {
      const request = store.getAll();

      request.onsuccess = (event) => {
        const recordingsData = (event.target as IDBRequest).result;
        const recordings: Recording[] = recordingsData.map((recordingData: any) => {
          const recording = new Recording(recordingData.name);
          recording.recordingData = recordingData.recordingData.map((record: any) => new Record(record.dataFlow, record.data));
          return recording;
        });
        resolve(recordings);
      };

      request.onerror = (event) => {
        reject((event.target as IDBRequest).error);
      };
    });
  }

  private async getUniqueRecordingName(name: string, store: IDBObjectStore): Promise<string> {
    let newName = name;
    let index = 1;

    if (name == "")
      name = "UnnamedRecording";

    while (await this.recordingNameExists(newName, store)) {
      newName = `${name}_${index++}`;
    }

    return newName;
  }

  private async recordingNameExists(name: string, store: IDBObjectStore): Promise<boolean> {
    return new Promise<boolean>((resolve, reject) => {
      const request = store.get(name);

      request.onsuccess = () => {
        resolve(!!request.result); // Resolve to true if the recording exists
      };

      request.onerror = () => {
        reject(request.error);
      };
    });
  }

  openDatabase() {
    return new Promise<IDBDatabase>((resolve, reject) => {
      const request = indexedDB.open(dbName, 1);

      request.onupgradeneeded = (event) => {
        const db = (event.target as IDBOpenDBRequest).result;
        if (!db.objectStoreNames.contains(storeName)) {
          db.createObjectStore(storeName);
        }
      };

      request.onsuccess = (event) => {
        resolve((event.target as IDBOpenDBRequest).result);
      };

      request.onerror = (event) => {
        reject((event.target as IDBOpenDBRequest).error);
      };
    });
  }
}

