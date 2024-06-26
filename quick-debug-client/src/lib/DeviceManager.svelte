<script lang="ts">
	import Input from "./Input.svelte";
	import { ConnectionState, IpData, Settings } from "./entities";
	import { IpDataStore, chartContext, freezePlotting, messageStore, ipDataStore, plottingInterval, configKeys } from "./store";

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


	let colorIdx = 0;
	let ipInputField: Input;
	let input: string;

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

	const maxPoints = $plottingInterval;

	function isIpValid(ipaddress: string) {
		if (ipaddress.match(/^(?:[0-9]{1,3}\.){3}[0-9]{1,3}$/)) {
			return true;
		} else if (ipaddress.match(/^(?:[0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$/)) {
			return true;
		} else {
			return false;
		}
	}

	function addIp() {
		var ip = input
		if (!isIpValid(ip)) {
			ipInputField.setError("Entered IP Address not valid!");
			return;
		}

		ipInputField.clearError();
		let data = new IpData(ip, false, ConnectionState.Disconnected);
		ipDataStore.addIp(data);
		connectWebSocket(data);
	}


	function connectWebSocket(data: IpData) {
		console.log("Connecting to " + data.IpAddress);
		data.ConnectionState = ConnectionState.Connecting
		ipDataStore.update();

		// if (data.Socket) 
		// 	data.Socket.close();

		data.Socket = new WebSocket(
			"ws://" + data.IpAddress + ":" + Settings.Port,
		);

		data.Socket.onopen = function () {
			console.log("Connected to " + data.IpAddress);
			data.ConnectionState = ConnectionState.Connected;
			ipDataStore.update();
		};

		data.Socket.onerror = function () {
			console.log("Error while connecting to " + data.IpAddress);
			data.ConnectionState = ConnectionState.Disconnected;
			ipDataStore.update();
		};
		
		data.Socket.onclose = function() {
			console.log("Connection closed: " + data.IpAddress);
			data.ConnectionState = ConnectionState.Disconnected;
			ipDataStore.update();
		}

		enum MessageType {
			Plot = "1",
			ConfigurationVariables = "2",
		}

		const messageCounter = { value: 0 };
		data.Socket.onmessage = function (event) {
			const message = event.data;
			const data = message.split(";");
			const messageType = data[0];

			if (messageType === MessageType.Plot)
				processPlotMessage(data, messageCounter);
			else if (messageType === MessageType.ConfigurationVariables)
				processsConfigMessage(data);
		};
	}

	
	function processPlotMessage(data: string[], messageCounter: any) {
		const field = data[1];
		const value = parseFloat(data[2]);
		if (!(!isNaN(value) && isFinite(value)))
			return;

		if ($freezePlotting)
			return;
		if ($chartContext == null || $chartContext.ChartSurface == null || $chartContext.WasmContext == null) {
			console.log("Chart context is null");
			return;
		}

		const sciChartSurface = $chartContext.ChartSurface;
		if (!$messageStore.has(field)) {
			console.log($chartContext.WasmContext)
			var series = new XyDataSeries($chartContext.WasmContext, {
				dataSeriesName: field,
				fifoCapacity: maxPoints,
				isSorted: true,
				containsNaN: false,
			}) 
			messageStore.update((x) => {
				x.set(
					field,
					series 
				);
				return x;
			});
			sciChartSurface.renderableSeries.add(
				new FastLineRenderableSeries($chartContext.WasmContext, {
					stroke: colorMap[colorIdx++ % colorMap.length],
					strokeThickness: 5,
					dataSeries: $messageStore.get(field),
					
				})
			);
		}

		sciChartSurface.zoomExtentsY();
		if (sciChartSurface.zoomState !== EZoomState.UserZooming) {
			sciChartSurface.xAxes.items[0].visibleRange = new NumberRange(messageCounter.value - maxPoints, messageCounter.value);
		}

		const dataSeries = $messageStore.get(field);
		dataSeries.append(messageCounter.value++, value);
	}

	function processsConfigMessage(data: string[]) {
		console.log("Available configuration variables:")
		for (let i = 1; i < data.length; i++) {
			const variable = data[i];

			configKeys.set([...$configKeys, variable]);
		}

		console.log($configKeys)
	}
</script>

<div class="row top-padding left-padding right-padding no-space no-margin">
	<Input
		bind:this={ipInputField}
		bind:input
		classes="max left-round border small"
		on:submit={addIp}
	/>
	<button
		class="small-round no-padding no-margin right-round small-round"
		on:click={addIp}
	>
		<i>add</i>
	</button>
</div>

<div class="divider no-margin"></div>

<div class="left-margin right-margin responsive">
	{#each $ipDataStore as ipData}
		<div
			class="row primary-border border small-round small-padding context-menu"
		>
			<!-- <label class="checkbox">
				<input
					type="checkbox"
					bind:checked={ipData.IsEnabled}
					id="x"
				/>
				<span></span>
			</label> -->
			<p class="max large-text left-margin" id="ip-address">
				{ipData.IpAddress}
			</p>
			{#if ipData.ConnectionState == ConnectionState.Connected}
				<div>
					<i>power</i>
				</div>
			{:else if ipData.ConnectionState == ConnectionState.Connecting}
				<div>
					<i class="spin">refresh</i>
				</div>
			{:else if ipData.ConnectionState == ConnectionState.Disconnected}
				<div
					on:click={() => connectWebSocket(ipData)}
					on:keypress={() => {}}
					role="button"
					tabindex="0"
				>
					<i style="color: red">power_off</i>
					<div class="tooltip">Reconnect</div>
				</div>
			{/if}
		</div>
	{/each}
</div>

<style>
	@keyframes spin {
		from {
			transform: rotate(0deg);
		}
		to {
			transform: rotate(360deg);
		}
	}

	.spin {
		animation: spin 1s linear infinite;
	}
</style>
