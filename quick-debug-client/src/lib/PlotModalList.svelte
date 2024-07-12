<script lang="ts">
  import { createEventDispatcher } from "svelte";
  import type { DataFlow } from "./entities/Entities";
  import type { Writable } from "svelte/store";

  export let dataFlowChartMap: Writable<Map<string, number>>;
  export let chartIdx: number;

  const dispatch = createEventDispatcher();

  function handleDragStart(event: DragEvent, flowName: string) {
    if (event.dataTransfer) {
      event.dataTransfer.effectAllowed = "move";
      event.dataTransfer.setData("text/plain", JSON.stringify(flowName));
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

    const dataFlowName = JSON.parse(dataReceived);
    dispatch("dataFlowDropped", dataFlowName);
  }
</script>

<!-- svelte-ignore a11y-no-static-element-interactions -->
<!-- svelte-ignore missing-declaration -->
<div
  class="content"
  on:dragover={handleDragOver}
  on:drop={(event) => handleDrop(event)}
>
  {#each Array.from($dataFlowChartMap) as [flowName, idx], index}
    {#if idx === chartIdx}
      <div
        draggable="true"
        class="flow-card max primary-border border fill small-margin wave"
        on:dragstart={(event) => handleDragStart(event, flowName)}
      >
        <p class="center-align">{flowName}</p>
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
