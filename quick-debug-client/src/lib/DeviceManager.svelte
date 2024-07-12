<script lang="ts">
  import Input from "./Input.svelte";
  import { ConnectionState, IpData } from "./entities/IpDataStore";
  import { Settings } from "./entities/Entities";
  import {
    freezePlotting,
    ipDataStore,
    configKeys,
    chartManager,
  } from "./entities/Store";

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
    var ip = input;
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
    data.ConnectionState = ConnectionState.Connecting;
    ipDataStore.update();

    data.Socket = new WebSocket("ws://" + data.IpAddress + ":" + Settings.Port);

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

    data.Socket.onclose = function () {
      console.log("Connection closed: " + data.IpAddress);
      data.ConnectionState = ConnectionState.Disconnected;
      ipDataStore.update();
    };

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
    if (!(!isNaN(value) && isFinite(value))) return;

    if ($freezePlotting) return;

    chartManager.plot(field, value);
  }

  function processsConfigMessage(data: string[]) {
    console.log("Available configuration variables:");
    for (let i = 1; i < data.length; i++) {
      const variable = data[i];

      configKeys.set([...$configKeys, variable]);
    }

    console.log($configKeys);
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
