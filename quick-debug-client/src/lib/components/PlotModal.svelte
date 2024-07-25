<script lang="ts">
  import PlotModalList from "@comps/PlotModalList.svelte";
  import { chartManager } from "@ents/Store";
  import { Chart } from "@ents/ChartManager";

  let chartsWith0: Chart[] = [];
  $: charts = chartManager.Charts;
  $: {
    chartsWith0 = Array.from($charts);
    chartsWith0.splice(0, 0, new Chart());
  }

  function addChart() {
    chartManager.requestNewChart();
  }

  function removeChart(idx: number) {
    chartManager.removeChart(idx);
  }

  function handleDataFlowDropped(event: CustomEvent, newChartId: number) {
    chartManager.moveDataFlow(event.detail, newChartId);
  }
</script>

<div class="overlay blur"></div>
<dialog id="chart-plotting-modal" class="small-round">
  <div class="row">
    <h5>Data-Flow Assignment</h5>
    <div class="max"></div>
    <button class="transparent link circle" data-ui="#chart-plotting-modal"
      ><i>close</i>
    </button>
  </div>

  <div class="row center-align responsive" style="align-items: stretch;">
    {#each chartsWith0 as chart, i}
      {#if i === 0}
        <div
          class="chart-container small-round secondary-container small-padding"
        >
          <h6 class="center-align small-margin">Not assigned</h6>
          <div class="divider secondary"></div>
          <PlotModalList
            dataFlowChartMap={chartManager.DataFlowChartMap}
            chartIdx={i}
            on:dataFlowDropped={(e) => handleDataFlowDropped(e, i)}
          ></PlotModalList>
        </div>
      {:else}
        <div
          class="chart-container small-round secondary-container small-padding"
        >
          <div class="row">
            <h6 class="center-align small-margin">Chart {i}</h6>
            <div class="max"></div>
            <button
              class="transparent link circle"
              on:click={() => removeChart(i)}
              ><i>close</i>
            </button>
          </div>
          <div class="divider secondary"></div>
          <PlotModalList
            dataFlowChartMap={chartManager.DataFlowChartMap}
            chartIdx={i}
            on:dataFlowDropped={(e) => handleDataFlowDropped(e, i)}
          ></PlotModalList>
        </div>
      {/if}
    {/each}
  </div>
  <div class="max">
    <button class="max small-round center-align center" on:click={addChart}>
      <i>add</i>
      Add new chart
    </button>
  </div>
</dialog>

<style>
  .chart-container {
    display: flex;
    flex-direction: column;
    width: 200px;
    min-height: 400px;
  }
</style>
