<script lang="ts">
  import { type Record, type Recording } from "@ents/RecordingManager";
  import { recordingManager } from "@ents/Store";

  class SelectableDataFlows {
    entries: Array<[name: string, count: number, selected: boolean]>;
    total: number;

    constructor(entries: Map<string, number>, total: number) {
      this.entries = Array.from(entries.entries()).map(([name, count]) => [
        name,
        count,
        false,
      ]);
      this.total = total;
    }
  }

  let selectedRecording: Recording | null = null;
  let selectedDataFlows: SelectableDataFlows = new SelectableDataFlows(
    new Map<string, number>(),
    0
  );
  $: recordings = recordingManager.recordings;
  $: {
    if (selectedRecording == null) {
      selectedDataFlows = new SelectableDataFlows(new Map<string, number>(), 0);
    } else {
      let returnedStats = selectedRecording!.gatherStats();
      selectedDataFlows = new SelectableDataFlows(
        returnedStats.entries,
        returnedStats.total
      );
    }
  }

  function onRecordingSelected() {
    if (selectedRecording == null) {
      return;
    }
  }

  function onRecordingFlowSelected(dataFlow: string) {
    let index = selectedDataFlows.entries.findIndex(
      ([name, count]) => name === dataFlow
    );
    selectedDataFlows.entries[index][2] = !selectedDataFlows.entries[index][2];
  }

  function deleteRecording() {
    if (selectedRecording == null) {
      return;
    }

    recordingManager.deleteRecording(selectedRecording.name);
    selectedRecording = null;
  }

  function downloadRecording(
    recording: Recording,
    selectedDataFlows: string[]
  ) {
    const csvContent = recording.convertToCSV(selectedDataFlows);
    const blob = new Blob([csvContent], { type: "text/csv;charset=utf-8;" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = recording.name + ".csv";
    link.click();
    URL.revokeObjectURL(url); // Clean up the URL object
  }

  function downloadSelectedRecording() {
    if (selectedRecording == null) {
      return;
    }

    const selected = selectedDataFlows.entries
      .filter(([_, __, selected]) => selected)
      .map(([dataFlow, _, __]) => dataFlow);

    downloadRecording(selectedRecording, selected);
  }

  function downloadAllRecordings() {
    let i = 0;
    for (let recording of $recordings) {
      setTimeout(() => {
        downloadRecording(recording, []);
      }, i++ * 200);
    }
  }

  function deleteAll() {
    if (window.confirm("Are you sure you want to delete all recordings?")) {
      recordingManager.deleteAllRecordings();
    }
  }
</script>

<div class="overlay blur"></div>
<dialog id="recording-modal" class="small-round large">
  <div class="row">
    <h5>Recordings</h5>

    <button class="small-round" on:click={downloadAllRecordings}>
      <i style="transform: scale(0.5);">download</i>
      <i style="margin-left: -32px; transform: scale(1.2)">folder</i>
      <span>Download All</span>
    </button>
    <button class="small-round" on:click={deleteAll}
      ><i>delete</i>
      <span>Delete All</span>
    </button>

    <div class="max"></div>
    <button class="transparent link circle" data-ui="#recording-modal"
      ><i>close</i>
    </button>
  </div>

  <div class="row center-align responsive" style="align-items: stretch;">
    <div class="field label suffix border max" style="min-width: 300px;">
      <select
        id="recording-select"
        bind:value={selectedRecording}
        on:change={() => onRecordingSelected()}
      >
        <option value={null}>Select</option>
        {#each $recordings as recording}
          <option value={recording}>{recording.name}</option>
        {/each}
      </select>
      <label for="recording-select">Recording</label>
      <i>arrow_drop_down</i>
    </div>
  </div>
  <div class="divider"></div>

  <!-- Display the stats of the selected recording -->
  <div
    class="row center-align responsive no-gap"
    style="display: grid; grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));"
  >
    {#each selectedDataFlows.entries as [dataFlow, count, selected]}
      <div class="vertical no-margin max">
        <!-- svelte-ignore a11y-click-events-have-key-events -->
        <!-- svelte-ignore a11y-no-noninteractive-element-interactions -->
        <article
          on:click={() => onRecordingFlowSelected(dataFlow)}
          class="data-flow-entry"
          class:selected
        >
          <p>{dataFlow}</p>
          <small>{count}</small>
          <progress class="max" value={count} max={selectedDataFlows.total}
          ></progress>
        </article>
      </div>
    {/each}
  </div>

  <div class="row center-align responsive">
    <button
      class="small-round"
      disabled={selectedRecording == null}
      on:click={downloadSelectedRecording}
    >
      <i>download</i>
      <span>Download</span>
    </button>
    <button
      class="small-round"
      disabled={selectedRecording == null}
      on:click={deleteRecording}
    >
      <i>delete</i>
      <span>Delete</span>
    </button>
  </div>
</dialog>

<style>
  .data-flow-entry {
    cursor: pointer;
    border: 1px solid transparent;
  }

  .data-flow-entry:hover {
    border-color: var(--primary);
  }

  .selected {
    background-color: var(--primary-container);
  }
</style>
