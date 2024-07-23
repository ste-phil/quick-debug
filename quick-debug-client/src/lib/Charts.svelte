<script lang="ts">
  import { onMount, tick } from "svelte";
  import { chartManager } from "./entities/Store";
  import { Chart, ChartType } from "./entities/ChartManager";

  let elementRefs: HTMLElement[] = [];
  let selectedButton: number = 1;

  onMount(() => {
    chartManager.init();
  });

  $: charts = chartManager.Charts;
  $: checkRenderedElements($charts);

  function resetFocus(chart: Chart) {
    const chartContext = chart.ChartContext!;
    chartContext.ChartSurface.zoomExtents();
  }

  function changeChartType(chart: Chart, chartType: ChartType, btnIdx: number) {
    chartManager.changeChartType(chart, chartType);
    selectedButton = btnIdx;
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

        <nav class="no-space">
          <button
            class="border left-round small-round {selectedButton === 0
              ? 'fill'
              : ''}"
            on:click={() => changeChartType(chart, ChartType.StackedColumn, 0)}
          >
            <i>stacked_bar_chart</i>
          </button>
          <button
            class="border right-round small-round {selectedButton === 1
              ? 'fill'
              : ''}"
            on:click={() => changeChartType(chart, ChartType.Line, 1)}
          >
            <i>show_chart</i>
          </button>
        </nav>

        <button
          class="small-round circle transaprent border"
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
  }
</style>
