import { writable, type Writable } from "svelte/store";
import { ChartContext, ConnectionState, IpData } from "./entities";

export class IpDataStore {
    private readonly trackedIps: Writable<IpData[]>;

    constructor() { 
        this.trackedIps = ClearConnectionState(UseLocalStorage("tracked-ips", [
            new IpData("192.168.1.2", true, ConnectionState.Disconnected),
            new IpData("192.168.3.123", true, ConnectionState.Connecting	),
            new IpData("2001:0:130F::09C0:876A:130B", false, ConnectionState.Connected),
        ]));
    }

    [Symbol.iterator]() {
        let index = 0;
        let values: IpData[] = [];
        this.subscribe((value) => {
          values = Array.isArray(value) ? value : [];
        })();
    
        return {
          next: () => {
            if (index < values.length) {
              return { value: values[index++], done: false };
            } else {
              index = 0;
              return { done: true };
            }
          }
        };
      }

    public addIp(data: IpData) {
        this.trackedIps.update(x => [...x, data]);
    }

    public removeIp(ip: string) {
        this.trackedIps.update(ips => {
            let elemToRemove = ips.find(x => x.IpAddress === ip)
            if (elemToRemove != undefined) {
                ips.splice(ips.indexOf(elemToRemove), 1);
                elemToRemove.Socket?.close();
                return ips;
            }
            return ips;
        });
    }

    public subscribe(val: (value: any) => void) {
        return this.trackedIps.subscribe(val);
    }

    public update() {
        this.trackedIps.update(x => x);
    }

    public Get(): Writable<IpData[]> {
        return this.trackedIps;
    }
}


export const ipDataStore: IpDataStore = new IpDataStore()

export const freezePlotting = writable(false);
export const darkMode = UseLocalStorage("dark-mode", true);
export const plottingInterval = UseLocalStorage("plotting-interval", 100);

export const messageStore: Writable<Map<string, XyDataSeries>> = writable(new Map());
export const chartContext: Writable<ChartContext | null> = writable(null);

function ClearConnectionState(store: Writable<IpData[]> ) {
    store.update(x => {
        x.forEach(ip => ip.ConnectionState = ConnectionState.Disconnected);
        return x;
    });

    return store;
}


function UseLocalStorage<T>(key: string, initial: T){
    const value = localStorage.getItem(key)
    const store = writable(value == null ? initial : JSON.parse(value));
    store.subscribe(v => localStorage.setItem(key, JSON.stringify(v)));
    
    return store;
}
