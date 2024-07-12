<script lang="ts">
  import { onMount, tick } from "svelte";
  import { chartManager } from "./entities/Store";
  import { Chart } from "./entities/ChartManager";

  let elementRefs: HTMLElement[] = [];

  onMount(() => {
    chartManager.init();
  });

  $: charts = chartManager.Charts;
  $: checkRenderedElements($charts);

  async function checkRenderedElements($charts: Chart[]) {
    await tick(); // Wait for the DOM to update
    if (elementRefs.length === $charts.length) {
      for (let i = 0; i < elementRefs.length; i++) {
        chartManager.createChart(i, elementRefs[i].id);
      }
    }
  }
</script>

<!-- <div id="chart" class="responsive"></div> -->
<div class="grid">
  {#each $charts as chart, index (chart)}
    <div class="s6 border small-round padding">
      <p>Chart {index}</p>
      <div bind:this={elementRefs[index]} id="chart-{index}"></div>
    </div>
  {/each}
</div>

<style>
  :global(.scichart__legend > div) {
    border: 1px solid #6750a4;
  }
</style>
