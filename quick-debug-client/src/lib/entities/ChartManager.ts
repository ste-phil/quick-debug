import { writable, type Writable } from "svelte/store";
import { chartContext, darkMode, plottingInterval } from "./Store";
import { UseLocalStorage, ReadWritable } from "./Utils";

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
	type TSciChart
} from "scichart";


const colorMap = [
	"#00FF00",
	"#0000FF",
	"#FF00FF",
	"#00FFFF",
	"#FFFF00",
	"#6600FF",
	"#FF6600",
	"#FF0000",
	"#00FF00",
	"#0000FF",
	"#FF00FF",
	"#00FFFF",
	"#FFFF00",
	"#6600FF",
];
let colorIdx = 0;

export class ChartContext {
	public WasmContext: TSciChart;
	public ChartSurface: SciChartSurface;

	constructor(wasmContext: any, chartSurface: any) {
		this.WasmContext = wasmContext;
		this.ChartSurface = chartSurface;
	}
}

export class Chart {
	public ChartContext: ChartContext | null = null;
	public HtmlElementId: string | null = null;

	public IsCreated() { return this.ChartContext !== null; }
}

export class ChartManager {
	private readonly charts: Writable<Chart[]>;
	private readonly chartMap: Writable<Map<string, number>>;

	public get Charts() { return this.charts; }
	public get DataFlowChartMap() { return this.chartMap; }

	constructor() {
		this.charts = writable([]);
		this.chartMap = UseLocalStorage<Map<string, number>>("ChartManager_chartMap", new Map<string, number>());
	}

	public init() {
		this.chartMap.update((x) => {
			for (const [key, value] of x) {
				if (value === undefined)
					continue;

				const chartContext = ReadWritable(this.charts)[value].ChartContext;
				if (chartContext === undefined) {
					this.requestNewChart().then(newChartContext => {
						const series = this.createSeries(key, newChartContext, $plottingInterval);
						this.createChart(newChartContext);
						this.addSeries(key, value)

					});
				}
				else {
					const series = this.createSeries(key, chartContext, $plottingInterval);
					this.addSeries(key, value)
				}
			}

			return x;
		});
	}

	public registerDataFlow(name: string) {
		ReadWritable(this.chartMap).set(name, 0);
	}

	public plot(graph: string, value: number) {
		const val = ReadWritable(this.chartMap).get(graph);
		if (val !== undefined) {

		}
	}

	public requestNewChart() {
		this.charts.update(x => [...x, new Chart()]);
	}

	public removeChart(chart: ChartContext) {
		this.Charts.update(x => {
			x.splice(x.indexOf(chart), 1);
			return x;
		});
	}

	public async addSeries(graph: string, chartIdx: number) {
		// if (this.$charts[chartIdx] === undefined) {
		// 	const newChartContext = await this.createChart();
		// 	this.addChart(newChartContext);
		// 	this.createSeries(graph, newChartContext, $plottingInterval);
		// }

		this.chartMap.update(x => {
			x.set(graph, chartIdx);
			return x;
		});
	}

	public removeSeries(graph: string) {
		this.chartMap.update(x => {
			x.delete(graph);
			return x;
		});
	}

	public async createChart(htmlElementId: string): Promise<ChartContext | null> {
		//Check if chart already exists
		if (ReadWritable(this.charts).find(x => x.HtmlElementId === htmlElementId) !== undefined) {
			return null;
		}


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
			}

			var theme = { ...new SciChartJsNavyTheme(), ...customTheme };
			return theme;
		}

		const { sciChartSurface, wasmContext } = await SciChartSurface.create(
			htmlElementId,
			{
				theme: createChartTheme(ReadWritable(darkMode)),
			},
		);

		chartContext.set(new ChartContext(
			wasmContext,
			sciChartSurface
		));

		const mouseWheelZoomModifier = new MouseWheelZoomModifier();
		// const rubberBandZoomModifier = new RubberBandXyZoomModifier();
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

		darkMode.subscribe(x => {
			sciChartSurface.applyTheme(createChartTheme(x))
		});

		console.log("Chart creating ...", ReadWritable(this.Charts));

		const chart = ReadWritable(this.charts).at(-1);
		if (chart === undefined) {
			console.error("Chart not found");
			return null;
		}

		chart.ChartContext = new ChartContext(wasmContext, sciChartSurface);
		chart.HtmlElementId = htmlElementId;

		console.log("Chart created", ReadWritable(this.Charts));

		return chart.ChartContext;
	}


	private createSeries(graph: string, chartContext: ChartContext, maxPoints: number): FastLineRenderableSeries {
		const series = new XyDataSeries(chartContext.WasmContext, {
			dataSeriesName: graph,
			fifoCapacity: maxPoints,
			isSorted: true,
			containsNaN: false,
		});

		const fastRenderSeries = new FastLineRenderableSeries(chartContext.WasmContext, {
			stroke: colorMap[colorIdx++ % colorMap.length],
			strokeThickness: 5,
			dataSeries: series,
		});

		chartContext.ChartSurface.renderableSeries.add(fastRenderSeries);

		// chartContext.ChartSurface.renderableSeries.asArray()[0].


		return fastRenderSeries;
	}
}