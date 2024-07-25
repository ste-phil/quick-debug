import { writable, type Writable } from "svelte/store";
import { darkMode, plottingInterval } from "./Store";
import { ReadWritable, UseLocalStorageMap } from "./Utils";

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
import { tick } from "svelte";


const colorMap = [
	"#009F00",
	"#0000FF",
	"#FF00FF",
	"#001FFF",
	"#CF8E00",
	"#6600FF",
	"#FF6600",
	"#FF0000",
	"#560061",
	"#304d00",
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

export class SeriesData {
	public Series: FastLineRenderableSeries;
	public AxisPlotPoint: number;

	constructor(series: FastLineRenderableSeries) {
		this.Series = series;
		this.AxisPlotPoint = 0;
	}

}

export class Chart {
	public ChartContext: ChartContext | null = null;
	public HtmlElementId: string | null = null;
	public Series: SeriesData[] = [];
	public AxisFurthestPlotPoint: number = 0;

	public IsCreated() { return this.ChartContext !== null; }
}

export class ChartManager {
	private readonly charts: Writable<Chart[]>;
	private readonly chartMap: Writable<Map<string, number>>;
	private readonly alltimeRegisteredDataFlowsMap: Writable<Map<string, number>>;

	public get Charts() { return this.charts; }
	public get DataFlowChartMap() { return this.chartMap; }

	constructor() {
		this.charts = writable([]);
		this.alltimeRegisteredDataFlowsMap = UseLocalStorageMap<string, number>("ChartManager_inactiveChartMap", new Map<string, number>([]));
		this.chartMap = writable<Map<string, number>>(new Map<string, number>([]));
	}

	public async init() {
		SciChartSurface.UseCommunityLicense();

		const storedChartMap = ReadWritable(this.chartMap);
		for (const [key, value] of storedChartMap) {
			if (value === undefined)
				continue;
			// Skip index 0 since it means this data-flow is not assigned to any chart
			if (value === 0)
				continue;

			const chart = ReadWritable(this.charts)[value - 1];
			if (chart === undefined) {
				console.log(`Chart with index ${value} not found. Requesting new chart.`);
				this.requestNewChart();
				await tick();
				continue;
			}
			const chartContext = chart.ChartContext;
			if (chartContext === null) {
				// Chart creation was already requested, but not yet completed
				return;
			}
			else {
				const series = this.createSeries(key, chart, ReadWritable(plottingInterval));
			}
		}
	}

	public plot(dataFlow: string, value: number) {
		const chartIdx = ReadWritable(this.chartMap).get(dataFlow);
		if (chartIdx === undefined) {
			this.addDataFlow(dataFlow);
			this.plot(dataFlow, value);
			return;
		}

		const chart = ReadWritable(this.charts)[chartIdx - 1];
		if (chart === undefined || chart.ChartContext === null)
			return;


		const series = chart.Series.find(x => x.Series.dataSeries.dataSeriesName === dataFlow);
		if (series === undefined) {
			this.createSeries(dataFlow, chart, ReadWritable(plottingInterval));
			this.plot(dataFlow, value);
			return;
		}

		// Update the axis plot point for this series and the chart
		// This is used to plot new data for different data flows at the same x point
		series.AxisPlotPoint++
		const newPlotPoint = Math.max(series.AxisPlotPoint, chart.AxisFurthestPlotPoint);
		chart.AxisFurthestPlotPoint = newPlotPoint;
		series.AxisPlotPoint = newPlotPoint;

		const xySeries = series.Series.dataSeries as XyDataSeries;
		xySeries.append(series.AxisPlotPoint, value);

		const chartSurface = chart.ChartContext?.ChartSurface;
		if (chartSurface === undefined) {
			console.log(`The chart surface for the data-flow '${dataFlow}' is not available. Does the chart exist?`);
			return;
		}

		if (chartSurface.zoomState !== EZoomState.UserZooming) {
			chartSurface.zoomExtentsY();
			chartSurface.xAxes.get(0).visibleRange = new NumberRange(
				series.AxisPlotPoint - series.Series.dataSeries.fifoCapacity!,
				series.AxisPlotPoint
			);
		}
	}

	public addDataFlow(dataFlow: string) {
		let chartIdx = ReadWritable(this.alltimeRegisteredDataFlowsMap).get(dataFlow);
		if (chartIdx === undefined)
			chartIdx = 0;

		// console.log(`Adding data-flow '${dataFlow}' to chart ${chartIdx}`);
		// console.log(`Current chart count is ${ReadWritable(this.Charts).length}`);
		while (chartIdx > ReadWritable(this.Charts).length) {
			this.requestNewChart();
		}

		this.chartMap.update(x => {
			x.set(dataFlow, chartIdx);
			return x;
		});

		this.alltimeRegisteredDataFlowsMap.update(x => {
			x.set(dataFlow, chartIdx);
			return x;
		});
	}

	public moveDataFlow(dataFlow: string, chartIdx: number) {
		this.chartMap.update(x => {
			const oldChartIdx = x.get(dataFlow);
			// Remove series from old chart if it exists and only if it is assigned 
			if (oldChartIdx !== undefined && oldChartIdx != 0) {
				this.removeSeries(ReadWritable(this.charts)[oldChartIdx - 1], dataFlow);
			}

			x.set(dataFlow, chartIdx);
			return x;
		});

		this.alltimeRegisteredDataFlowsMap.update(x => {
			x.set(dataFlow, chartIdx);
			return x;
		});

		const chart = ReadWritable(this.charts)[chartIdx];
		if (chart === undefined) {
			return;
		}
		this.createSeries(dataFlow, chart, ReadWritable(plottingInterval));
	}


	public removeDataFlow(dataFlow: string) {
		this.chartMap.update(x => {
			x.delete(dataFlow);
			return x;
		});
	}

	public requestNewChart() {
		this.charts.update(x => [...x, new Chart()]);
	}

	public removeChart(chartIdx: number) {
		chartIdx = Math.max(0, chartIdx - 1); // Since the index is 1-based
		this.removeDatFlowAssignmentsFromChart(chartIdx);
		this.charts.update(x => {
			x.splice(chartIdx, 1)
			return x;
		});
	}

	public async createChart(chartIdx: number, htmlElementId: string): Promise<ChartContext | null> {
		//Check if chart already exists
		if (ReadWritable(this.charts).find(x => x.HtmlElementId === htmlElementId) !== undefined) {
			return null;
		}

		console.log("Creating chart nr: " + ReadWritable(this.charts).length + " and element id: " + htmlElementId);

		// Create a custom theme for the chart
		const createChartTheme = (darkMode: boolean) => {
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

		const chart = ReadWritable(this.charts).at(chartIdx);
		if (chart === undefined) {
			console.error("Chart not found");
			return null;
		}

		chart.ChartContext = new ChartContext(wasmContext, sciChartSurface);
		chart.HtmlElementId = htmlElementId;

		return chart.ChartContext;
	}

	// Checks if any data-flows are aassigned to this chart and assigns them to the first
	private removeDatFlowAssignmentsFromChart(chartIdx: number) {
		this.chartMap.update(x => {
			for (const [key, value] of x) {
				const chart = ReadWritable(this.charts)[value];
				this.removeSeries(chart, key);

				if (value === chartIdx) {
					x.set(key, 0);
				}
			}
			return x;
		});
	}

	private createSeries(graph: string, chart: Chart, maxPoints: number): FastLineRenderableSeries {
		const chartContext = chart.ChartContext;
		if (chartContext === null) {
			throw new Error("ChartContext is null. This should not happen.");
		}

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
		chart.Series.push(new SeriesData(fastRenderSeries));

		return fastRenderSeries;
	}

	private removeSeries(chart: Chart, dataFlow: string) {
		const series = chart.Series.find(x => x.Series.dataSeries.dataSeriesName === dataFlow);
		if (series === undefined)
			return;

		// Remove fastrenderable series from actual chart
		chart.ChartContext?.ChartSurface.renderableSeries.remove(series.Series);
		// Remove series from series array
		chart.Series = chart.Series.filter(x => x.Series.dataSeries.dataSeriesName !== dataFlow);

		series.Series.delete();
	}
}
