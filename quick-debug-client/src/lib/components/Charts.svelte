<script lang="ts">
  import { onMount, tick } from "svelte";
  import { chartManager } from "lib/entities/Store";
  import { Chart } from "lib/entities/ChartManager";

  let elementRefs: HTMLElement[] = [];

  onMount(() => {
    chartManager.init();
  });

  $: charts = chartManager.Charts;
  $: checkRenderedElements($charts);

  function resetFocus(chart: Chart) {
    const chartContext = chart.ChartContext!;
    chartContext.ChartSurface.zoomExtents();
  }

  async function checkRenderedElements($charts: Chart[]) {
    await tick(); // Wait for the DOM to update
    if (elementRefs.length === $charts.length) {
      for (let i = 0; i < elementRefs.length; i++) {
        chartManager.createChart(i, elementRefs[i].id);
      }
    }
  }
</script>

<div class="grid">
  {#each $charts as chart, index (chart)}
    <div class="s6 border small-round padding">
      <div class="row">
        <p>Chart {index}</p>
        <div class="max"></div>
        <button
          class="transparent link circle"
          on:click={() => resetFocus(chart)}
        >
          <i>crop_free</i>
        </button>
      </div>
      <div bind:this={elementRefs[index]} id="chart-{index}"></div>
    </div>
  {/each}
</div>

<style>
  :global(.scichart__legend > div) {
    border: 1px solid #6750a4;
    z-index: 0; /* Fix for the legend being on top of the chart */
  }
  :global(svg:has(> .scichart__rollover-tooltip)) {
    z-index: 1;
  }
</style>
