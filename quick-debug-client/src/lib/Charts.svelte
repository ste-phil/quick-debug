<script lang="ts">
  import { onMount, tick } from "svelte";
  import { chartContext, darkMode, chartManager } from "./entities/Store";
  import { Chart, ChartContext } from "./entities/ChartManager";
  import {
    SciChartSurface,
    SciChartDefaults,
    chartBuilder,
    SciChartJsNavyTheme,
    XyDataSeries,
    FastLineRenderableSeries,
    NumericAxis,
    MouseWheelZoomModifier,
    RubberBandXyZoomModifier,
    ZoomExtentsModifier,
    RolloverModifier,
    LegendModifier,
    ZoomPanModifier,
    EZoomState,
    NumberRange,
    EExecuteOn,
    AUTO_COLOR,
  } from "scichart";
  import { writable } from "svelte/store";

  function createChartTheme(darkMode: boolean) {
    const customTheme = {
      //axisBorder: "Transparent",
      //axisTitleColor: "#6495ED",
      //annotationsGripsBackroundBrush: "white",
      //annotationsGripsBorderBrush: "white",
      //axis3DBandsFill: "#1F3D6833",
      axisBandsFill: darkMode ? "transparent" : "rgba(0, 0, 0, 0.05)",
      // axisPlaneBackgroundFill: "red",
      //columnFillBrush: "white",
      // columnLineColor: "#e1d7fd",
      cursorLineBrush: "red",
      // defaultColorMapBrush: [
      //    { offset: 0, color: "red" },
      //    { offset: 0.5, color: "CornflowerBlue" },
      //    { offset: 1, color: "#FF22AA" }
      // ],
      //downBandSeriesFillColor: "#52CC5490",
      //downBandSeriesLineColor: "#E26565FF",
      //downBodyBrush: "white",
      //downWickColor: "white",
      // gridBackgroundBrush: "red",
      // gridBorderBrush: "white",
      //labelBackgroundBrush: "#6495EDAA",
      //labelBorderBrush: "#6495ED",
      labelForegroundBrush: "black",
      legendBackgroundBrush: "white",
      //lineSeriesColor: "white",
      loadingAnimationBackground: "Transparent",
      loadingAnimationForeground: "Transparent",
      majorGridLineBrush: "transparent",
      minorGridLineBrush: "transparent",
      //mountainAreaBrush: "white",
      //mountainLineColor: "white",
      //overviewFillBrush: "white",
      //planeBorderColor: "white",
      // rolloverLineBrush: "white",
      //rubberBandFillBrush: "#99999933",
      //rubberBandStrokeBrush: "#99999977",
      sciChartBackground: "transparent",
      //scrollbarBackgroundBrush: "white",
      //scrollbarBorderBrush: "white",
      //scrollbarGripsBackgroundBrush: "white",
      //scrollbarViewportBackgroundBrush: "white",
      //scrollbarViewportBorderBrush: "white",
      shadowEffectColor: "transparent",
      // textAnnotationBackground: "#cdbdfb00",
      textAnnotationForeground: "#FF0000",
      tickTextBrush: darkMode ? "white" : "black",
      //upBandSeriesFillColor: "white",
      //upBandSeriesLineColor: "white",
      //upBodyBrush: "#6495EDA0",
      //upWickColor: "#6495ED"
    };

    var theme = { ...new SciChartJsNavyTheme(), ...customTheme };
    return theme;
  }

  async function initChart() {
    const { sciChartSurface, wasmContext } = await SciChartSurface.create(
      "chart",
      {
        theme: createChartTheme($darkMode),
      }
    );

    chartContext.set(new ChartContext(wasmContext, sciChartSurface));
    // sciChartSurface.applyTheme(customTheme)

    const mouseWheelZoomModifier = new MouseWheelZoomModifier();
    //const rubberBandZoomModifier = new RubberBandXyZoomModifier();
    const zoomPanModifier = new ZoomPanModifier({
      executeOn: EExecuteOn.MouseMiddleButton,
    });
    const zoomExtentsModifier = new ZoomExtentsModifier({
      isAnimated: true,
    });

    sciChartSurface.chartModifiers.add(zoomPanModifier);
    //sciChartSurface.chartModifiers.add(rubberBandZoomModifier);
    sciChartSurface.chartModifiers.add(mouseWheelZoomModifier);
    sciChartSurface.chartModifiers.add(zoomExtentsModifier);
    sciChartSurface.chartModifiers.add(
      new LegendModifier({ showCheckboxes: true })
    );
    sciChartSurface.chartModifiers.add(new RolloverModifier(wasmContext));

    sciChartSurface.xAxes.add(new NumericAxis(wasmContext));
    sciChartSurface.yAxes.add(new NumericAxis(wasmContext));

    darkMode.subscribe((x) => {
      sciChartSurface.applyTheme(createChartTheme(x));
    });

    return { sciChartSurface, wasmContext };
  }

  let elementRefs: HTMLElement[] = [];

  onMount(() => {
    SciChartSurface.UseCommunityLicense();
    initChart();

    chartManager.init();
  });

  $: charts = chartManager.Charts;
  $: checkRenderedElements($charts);

  async function checkRenderedElements($charts: Chart[]) {
    console.log("Checking rendered elements:", elementRefs, $charts);
    await tick(); // Wait for the DOM to update
    console.log("tick:", elementRefs, $charts);

    if (elementRefs.length === $charts.length) {
      console.log("All elements are rendered:", elementRefs);

      for (let i = 0; i < elementRefs.length; i++) {
        chartManager.createChart(elementRefs[i].id);
      }
    }
  }
</script>

<div id="chart" class="responsive"></div>
<div class="grid">
  {#each $charts as chart, index (chart)}
    <div class="s6">
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
