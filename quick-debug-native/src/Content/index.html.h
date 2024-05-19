namespace Content {
	constexpr const char* index_html = R"(
	<!DOCTYPE html>
	<html>
	<head>
		<title>DebugCharting</title>
	</head>
	<body>
	<script src="https://cdn.jsdelivr.net/npm/scichart/index.min.js" crossorigin="anonymous"></script>
	<input type="text" placeholder="ip-address" id="ip-input">
	<input type="button" value="Connect" id="connect-button">
	<div id="scichart-root"></div>

	</body>
	</html>

	<style>
		/* Add your styles here */
		#scichart-root {
			width: 100%;
			height: 400px;
		}
		body {
			background-color: #222222;
		}
	</style>

	<script>
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
        //columnLineColor: "white",
        //cursorLineBrush: "#b3b3b3",
        //defaultColorMapBrush: [
        //    { offset: 0, color: "DarkBlue" },
        //    { offset: 0.5, color: "CornflowerBlue" },
        //    { offset: 1, color: "#FF22AA" }
        //],
        //downBandSeriesFillColor: "#52CC5490",
        //downBandSeriesLineColor: "#E26565FF",
        //downBodyBrush: "white",
        //downWickColor: "white",
        //gridBackgroundBrush: "white",
        //gridBorderBrush: "white",
        //labelBackgroundBrush: "#6495EDAA",
        //labelBorderBrush: "#6495ED",
        labelForegroundBrush: "#EEEEEE",
        legendBackgroundBrush: "#1D2C35",
        //lineSeriesColor: "white",
        //loadingAnimationBackground: "#0D213A",
        //loadingAnimationForeground: "#6495ED",
        //majorGridLineBrush: "#1F3D68",
        //minorGridLineBrush: "#102A47",
        //mountainAreaBrush: "white",
        //mountainLineColor: "white",
        //overviewFillBrush: "white",
        //planeBorderColor: "white",
        rolloverLineBrush: "#b3b3b3",
        //rubberBandFillBrush: "#99999933",
        //rubberBandStrokeBrush: "#99999977",
        sciChartBackground: "transparent",
        //scrollbarBackgroundBrush: "white",
        //scrollbarBorderBrush: "white",
        //scrollbarGripsBackgroundBrush: "white",
        //scrollbarViewportBackgroundBrush: "white",
        //scrollbarViewportBorderBrush: "white",
        //shadowEffectColor: "white",
        //textAnnotationBackground: "#6495EDAA",
        //textAnnotationForeground: "#EEEEEE",
        //tickTextBrush: "#6495ED",
        //upBandSeriesFillColor: "white",
        //upBandSeriesLineColor: "white",
        //upBodyBrush: "#6495EDA0",
        //upWickColor: "#6495ED"
    }


	// Learn more about the programmatic API at
	// https://www.scichart.com/javascript-chart-documentation
	async function initSciChartProgrammaticApi() {
		const { sciChartSurface, wasmContext } = await SciChartSurface.create(
		"scichart-root",
			{
				theme: new SciChartJsNavyTheme()
			}
		);

		sciChartSurface.applyTheme(customTheme);

		const mouseWheelZoomModifier = new MouseWheelZoomModifier();
		//const rubberBandZoomModifier = new RubberBandXyZoomModifier();
		const zoomPanModifier = new ZoomPanModifier({ executeOn: EExecuteOn.MouseMiddleButton });   
		const zoomExtentsModifier = new ZoomExtentsModifier({isAnimated: true });
		

		sciChartSurface.chartModifiers.add(zoomPanModifier);
		//sciChartSurface.chartModifiers.add(rubberBandZoomModifier);
		sciChartSurface.chartModifiers.add(mouseWheelZoomModifier);
		sciChartSurface.chartModifiers.add(zoomExtentsModifier);
		sciChartSurface.chartModifiers.add(new LegendModifier({showCheckboxes: true}));
		sciChartSurface.chartModifiers.add(new RolloverModifier(wasmContext));

		sciChartSurface.xAxes.add(new NumericAxis(wasmContext));
		sciChartSurface.yAxes.add(new NumericAxis(wasmContext));


		return { sciChartSurface, wasmContext };
	}
	
	async function init() {
		const lineData = {};
		const fields = new Map();
		const { sciChartSurface, wasmContext } = await initSciChartProgrammaticApi();

		let socket = null;
		function connectWebSocket(ip) {
			if (socket)
				socket.close();

			socket = new WebSocket(
			  "ws://" + ip + ":8126"
			);

			const maxPoints = 100;
			let i = 0;
			socket.onmessage = function (event) {
				const message = event.data;
				const data = message.split(";");
				const field = data[0];
				const value = data[1] * 1;	
			
				if (!fields.has(field)) {
					fields.set(field, new XyDataSeries(wasmContext, {
						dataSeriesName: field
					}));
					sciChartSurface.renderableSeries.add(
						new FastLineRenderableSeries(wasmContext, {
							strokeThickness: 3,
							dataSeries: fields.get(field)
						})
					);
				}

			sciChartSurface.zoomExtentsY();
			if (sciChartSurface.zoomState !== EZoomState.UserZooming) {
				sciChartSurface.xAxes.items[0].visibleRange = new NumberRange(i - maxPoints, i);
			}

				fields.get(field).append(i++, value);	
			};
		}

		document.getElementById("connect-button").onclick = function() {
			const field = document.getElementById("ip-input");
			connectWebSocket(field.value);

			//iterate over all fields and clear them
			fields.forEach((value, key) => {
				value.clear();
			});
		};

		

		connectWebSocket("localhost");
	};

	init();

	</script>
	)";
}

