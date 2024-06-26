import { writable, type Invalidator, type Subscriber, type Unsubscriber, type Updater, type Writable } from "svelte/store";
import { ChartContext, ConfigMessage, ConnectionState, IpData } from "./entities";

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
	EExecuteOn,
	AUTO_COLOR
} = SciChart;


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

export class IpDataStore {
	private readonly trackedIps: Writable<IpData[]>;

	constructor() {
		this.trackedIps = ClearConnectionState(UseLocalStorage("tracked-ips", [
			new IpData("192.168.1.2", true, ConnectionState.Disconnected),
			new IpData("192.168.3.123", true, ConnectionState.Connecting),
			new IpData("2001:0:130F::09C0:876A:130B", false, ConnectionState.Connected),
		]));
	}

	[Symbol.iterator]() {
		let index = 0;
		let values: IpData[] = [];
		this.subscribe((value) => {
			values = Array.isArray(value) ? value : [];
		})();

		return {
			next: () => {
				if (index < values.length) {
					return { value: values[index++], done: false };
				} else {
					index = 0;
					return { done: true };
				}
			}
		};
	}

	public addIp(data: IpData) {
		this.trackedIps.update(x => [...x, data]);
	}

	public removeIp(ip: string) {
		this.trackedIps.update(ips => {
			let elemToRemove = ips.find(x => x.IpAddress === ip)
			if (elemToRemove != undefined) {
				ips.splice(ips.indexOf(elemToRemove), 1);
				elemToRemove.Socket?.close();
				return ips;
			}
			return ips;
		});
	}

	public subscribe(val: (value: any) => void) {
		return this.trackedIps.subscribe(val);
	}

	public update() {
		this.trackedIps.update(x => x);
	}

	public Get(): Writable<IpData[]> {
		return this.trackedIps;
	}

	public SendMessageToAll(msg: ConfigMessage) {
		for (const e of this) {
			if (e?.Socket?.send !== undefined)
				e?.Socket?.send(`${msg.Key};${msg.Value}`)
		}
	}
}


export class ChartManager {
	private readonly chartMap: Writable<Map<string, number>>;
	private readonly charts: Writable<ChartContext[]>;

	public get Charts(): Writable<ChartContext[]> { return this.charts; }

	constructor() {
		this.charts = writable([]);
		this.chartMap = UseLocalStorage<Map<string, number>>("graph-chart-map", new Map<string, number>());
	}

	public init() {
		this.chartMap.update((x) => {
			for (const [key, value] of x) {
				if (value === undefined)
					continue;

				const chartContext = this.$charts[value];
				if (chartContext === undefined) {
					this.createChart().then(newChartContext => {
						this.createSeries(key, newChartContext, $plottingInterval);
						this.addChart(chartContext);
					});
				}
				else {
					this.createSeries(key, chartContext, $plottingInterval);
					this.addChart(chartContext);
				}
			}

			return x;
		});
	}

	public addChart(chart: ChartContext) {
		this.Charts.update(x => [...x, chart]);
	}

	public removeChart(chart: ChartContext) {
		this.Charts.update(x => {
			x.splice(x.indexOf(chart), 1);
			return x;
		});
	}

	public async addSeries(graph: string, chartIdx: number) {

		if (this.$charts[chartIdx] === undefined) {
			const newChartContext = await this.createChart();
			this.addChart(newChartContext);
			this.createSeries(graph, newChartContext, $plottingInterval);
		}

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

	public plot(graph: string, value: number) {
		const val = this.chartMap.get(graph);
		if (val !== undefined) {

		}
	}

	private async createChart(): Promise<ChartContext> {
		function createChartTheme(darkMode: boolean) {
			const customTheme = {
					//axisBorder: "Transparent",
					//axisTitleColor: "#6495ED",
					//annotationsGripsBackroundBrush: "white",
					//annotationsGripsBorderBrush: "white",
					//axis3DBandsFill: "#1F3D6833",
					axisBandsFill: darkMode ? "transparent" : "rgba(0, 0, 0, 0.05)" ,
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
	
			var theme = {...new SciChartJsNavyTheme(), ...customTheme};
			return theme;
		}

		const { sciChartSurface, wasmContext } = await SciChartSurface.create(
			"chart",
			{
				theme: createChartTheme($darkMode)
			},
		);

		chartContext.set(new ChartContext(
			wasmContext,
			sciChartSurface
		));
		// sciChartSurface.applyTheme(customTheme)

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

		return new ChartContext(wasmContext, sciChartSurface);
	}
 

	private createSeries(graph: string, chartContext: ChartContext, maxPoints: number): any {
		const series = new XyDataSeries(chartContext.WasmContext, {
			dataSeriesName: graph,
			fifoCapacity: maxPoints,
			isSorted: true,
			containsNaN: false,
		});

		chartContext.ChartSurface.renderableSeries.add(
			new FastLineRenderableSeries(chartContext.WasmContext, {
				stroke: colorMap[colorIdx++ % colorMap.length],
				strokeThickness: 5,
				dataSeries: series,
			})
		);

		return series;
	}
}

export const chartManager: ChartManager = new ChartManager();
export const ipDataStore: IpDataStore = new IpDataStore()
export const configMessages: Writable<ConfigMessage[]> = UseLocalStorage("config-messages", [
	new ConfigMessage("Bitrate", "8000"),
]);
export const configKeys: Writable<string[]> = writable([]);

export const freezePlotting = writable(false);
export const darkMode = UseLocalStorage("dark-mode", true);
export const plottingInterval = UseLocalStorage("plotting-interval", 100);

export const messageStore: Writable<Map<string, XyDataSeries>> = writable(new Map());
export const chartContext: Writable<ChartContext | null> = writable(null);

function ClearConnectionState(store: Writable<IpData[]>) {
	store.update(x => {
		x.forEach(ip => ip.ConnectionState = ConnectionState.Disconnected);
		return x;
	});

	return store;
}

function UseLocalStorage<T>(key: string, initial: T) {
	const value = localStorage.getItem(key)
	const store = writable(value == null ? initial : JSON.parse(value));
	store.subscribe(v => localStorage.setItem(key, JSON.stringify(v)));

	return store;
}
