<script lang="ts">
  import { chartManager } from "./entities/Store.ts";
  import { DataFlow } from "./entities/Entities.ts";
  import PlotModalList from "./PlotModalList.svelte";
  import { writable } from "svelte/store";

  let numberCharts = [0, 1, 2];

  let dataFlows: Writable<DataFlow[]> = writable([
    new DataFlow("A", 1),
    new DataFlow("B", 2),
    new DataFlow("C", 2),
    new DataFlow("D", 0),
    new DataFlow("E", 1),
  ]);

  function AddChart() {
    chartManager.requestNewChart();
    numberCharts = [...numberCharts, numberCharts.length];
  }

  function RemoveChart(idx: number) {
    return () => {
      numberCharts = numberCharts.filter((_, i) => i !== idx);
    };
  }

  function HandleDataFlowDropped(event: CustomEvent, newChartId: number) {
    console.log(event.detail);

    dataFlows.update((x) => {
      return x.map((flow) => {
        if (flow.Name === event.detail.Name) {
          return new DataFlow(flow.Name, newChartId);
        }
        return flow;
      });
    });
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

  <div class="row">
    {#each numberCharts as item, i}
      {#if i === 0}
        <div
          class="chart-container small-round secondary-container small-padding"
        >
          <h6 class="center-align small-margin">Not assigned</h6>
          <div class="divider secondary"></div>
          <PlotModalList
            {dataFlows}
            chartIdx={i}
            on:dataFlowDropped={(e) => HandleDataFlowDropped(e, i)}
          ></PlotModalList>
        </div>
      {:else}
        <div
          class="chart-container small-round secondary-container small-padding"
        >
          <div class="row">
            <h6 class="center-align small-margin">Chart {i}</h6>
            <div class="max"></div>
            <button class="transparent link circle" on:click={RemoveChart(i)}
              ><i>close</i>
            </button>
          </div>
          <div class="divider secondary"></div>
          <PlotModalList
            {dataFlows}
            chartIdx={i}
            on:dataFlowDropped={(e) => HandleDataFlowDropped(e, i)}
          ></PlotModalList>
        </div>
      {/if}
    {/each}
  </div>
  <div class="max">
    <button class="max small-round center-align center" on:click={AddChart}>
      <i>add</i>
      Add new chart
    </button>
  </div>
</dialog>

<style>
  .chart-container {
    width: 200px;
    height: 400px;
  }
</style>
