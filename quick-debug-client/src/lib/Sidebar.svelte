<script lang="ts">
    import { flip } from "svelte/animate";
	import Input from "./Input.svelte";
	import { ConnectionState, IpData, Settings } from "./entities";
    import { chartContext, freezePlotting, messageStore, trackedIps } from "./store";
    import { EZoomState, FastLineRenderableSeries, NumberRange, XyDataSeries } from "scichart";

	let ipInputField: Input;
	let input: string;

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
		trackedIps.update(x => [...x, data]);
		connectWebSocket(data);
	}

	function connectWebSocket(data: IpData) {
		console.log("Connecting to " + data.IpAddress);
		data.ConnectionState = ConnectionState.Connecting
		// trackedIps = trackedIps;
		trackedIps.update(x => x);

		// if (data.Socket) 
		// 	data.Socket.close();

		data.Socket = new WebSocket(
			"ws://" + data.IpAddress + ":" + Settings.Port,
		);

		data.Socket.onopen = function () {
			console.log("Connected to " + data.IpAddress);
			data.ConnectionState = ConnectionState.Connected;
			trackedIps.update(x => x);

		};

		data.Socket.onerror = function () {
			console.log("Error while connecting to " + data.IpAddress);
			data.ConnectionState = ConnectionState.Disconnected;
			trackedIps.update(x => x);
		};

		const maxPoints = 100;
		let i = 0;
		data.Socket.onmessage = function (event) {
			const message = event.data;
			const data = message.split(";");
			const field = data[0];
			const value = data[1] * 1;

			if (freezePlotting)
				return;
			console.log("Received: " + field + " = " + value);

			if ($chartContext == null) {
				console.log("Chart context is null");
				return;
			}

			const sciChartSurface = $chartContext.ChartSurface;
			if (!$messageStore.has(field)) {
				messageStore.update((x) => {
					x.set(field, new XyDataSeries($chartContext?.WasmContext));
					return x;
				});
				sciChartSurface.renderableSeries.add(
					new FastLineRenderableSeries($chartContext.WasmContext, {
						strokeThickness: 3,
						dataSeries: $messageStore.get(field)
					})
				);
			}

			sciChartSurface.zoomExtentsY();
			if (sciChartSurface.zoomState !== EZoomState.UserZooming) {
				sciChartSurface.xAxes.items[0].visibleRange = new NumberRange(i - maxPoints, i);
			}

			$messageStore.get(field).append(i++, value);
		};
	}
</script>

<div class="surface-container-low max">
	<div class="row padding padding-small no-space small-margin">
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

	<div class="margin">
		{#each $trackedIps as ipData}
			<div
				class="row primary-border border small-round small-padding context-menu"
			>
				<label class="checkbox">
					<input
						type="checkbox"
						bind:checked={ipData.IsEnabled}
						id="x"
					/>
					<span></span>
				</label>
				<p class="max large-text" id="ip-address">
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
