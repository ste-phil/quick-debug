<script lang="ts">
    import { onMount } from "svelte";
    import { chartContext } from "./store";
    import { ChartContext } from "./entities";

    const {
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
		EExecuteOn
	} = SciChart;

	const customTheme = {
        //axisBorder: "Transparent",
        //axisTitleColor: "#6495ED",
        //annotationsGripsBackroundBrush: "white",
        //annotationsGripsBorderBrush: "white",
        //axis3DBandsFill: "#1F3D6833",
        //axisBandsFill: "#1F3D6833",
        //axisPlaneBackgroundFill: "Transparent",
        //columnFillBrush: "white",
        // columnLineColor: "#e1d7fd",
        // cursorLineBrush: "white",
        defaultColorMapBrush: [
           { offset: 0, color: "red" },
           { offset: 0.5, color: "CornflowerBlue" },
           { offset: 1, color: "#FF22AA" }
        ],
        //downBandSeriesFillColor: "#52CC5490",
        //downBandSeriesLineColor: "#E26565FF",
        //downBodyBrush: "white",
        //downWickColor: "white",
        // gridBackgroundBrush: "white",
        // gridBorderBrush: "white",
        //labelBackgroundBrush: "#6495EDAA",
        //labelBorderBrush: "#6495ED",
        labelForegroundBrush: "#EEEEEE",
        legendBackgroundBrush: "#1D2C35",
        //lineSeriesColor: "white",
        loadingAnimationBackground: "Transparent",
        loadingAnimationForeground: "Transparent",
        //majorGridLineBrush: "#1F3D68",
        //minorGridLineBrush: "#102A47",
        //mountainAreaBrush: "white",
        //mountainLineColor: "white",
        //overviewFillBrush: "white",
        //planeBorderColor: "white",
        rolloverLineBrush: "white",
        //rubberBandFillBrush: "#99999933",
        //rubberBandStrokeBrush: "#99999977",
        sciChartBackground: "transparent",
        //scrollbarBackgroundBrush: "white",
        //scrollbarBorderBrush: "white",
        //scrollbarGripsBackgroundBrush: "white",
        //scrollbarViewportBackgroundBrush: "white",
        //scrollbarViewportBorderBrush: "white",
        //shadowEffectColor: "white",
        textAnnotationBackground: "#cdbdfb00",
        textAnnotationForeground: "black",
        //tickTextBrush: "#6495ED",
        //upBandSeriesFillColor: "white",
        //upBandSeriesLineColor: "white",
        //upBodyBrush: "#6495EDA0",
        //upWickColor: "#6495ED"
    }

	async function initSciChartProgrammaticApi() {
		const { sciChartSurface, wasmContext } = await SciChartSurface.create(
			"chart",
			{
				theme: new SciChartJsNavyTheme(),
			},
		);

		chartContext.set(new ChartContext(
            wasmContext,
            sciChartSurface
        ));

		sciChartSurface.applyTheme(customTheme);
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
			new LegendModifier({ showCheckboxes: true }),
		);
		sciChartSurface.chartModifiers.add(new RolloverModifier(wasmContext));

		sciChartSurface.xAxes.add(new NumericAxis(wasmContext));
		sciChartSurface.yAxes.add(new NumericAxis(wasmContext));

        return { sciChartSurface, wasmContext };
	}

	onMount(() => {
		initSciChartProgrammaticApi();
	});
</script>

<div>
	<h1>Chart</h1>
	<div id="chart" class="responsive"></div>
</div>