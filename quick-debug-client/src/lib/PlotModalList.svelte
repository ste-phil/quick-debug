<script lang="ts">
  import { createEventDispatcher } from "svelte";
  import type { DataFlow } from "./entities/Entities";
  import type { Writable } from "svelte/store";

  export let dataFlows: Writable<DataFlow[]>;
  export let chartIdx: number;

  const dispatch = createEventDispatcher();

  function handleDragStart(event: DragEvent, item: any) {
    if (event.dataTransfer) {
      event.dataTransfer.effectAllowed = "move";
      event.dataTransfer.setData("text/plain", JSON.stringify(item));
    }
  }

  function handleDragOver(event: DragEvent) {
    event.preventDefault();
    if (event.dataTransfer) {
      event.dataTransfer.dropEffect = "move";
    }
  }

  function handleDrop(event: DragEvent) {
    event.preventDefault();

    const dataReceived = event.dataTransfer?.getData("text/plain");
    if (!dataReceived) return;

    const obj = JSON.parse(dataReceived);
    dispatch("dataFlowDropped", obj);
  }
</script>

<!-- svelte-ignore a11y-no-static-element-interactions -->
<!-- svelte-ignore missing-declaration -->
<div
  class="content"
  on:dragover={handleDragOver}
  on:drop={(event) => handleDrop(event)}
>
  {#each $dataFlows as flow}
    {#if flow.AssignedChartIdx === chartIdx}
      <div
        draggable="true"
        class="flow-card max primary-border border fill small-margin wave"
        on:dragstart={(event) => handleDragStart(event, flow)}
      >
        <p class="center-align">{flow.Name}</p>
      </div>
    {/if}
  {/each}
</div>

<style>
  .content {
    width: 100%;
    height: 100%;
  }

  .flow-card {
    cursor: pointer;
    user-select: none;
    font-size: 16px;
    padding: 8px;
  }
</style>
