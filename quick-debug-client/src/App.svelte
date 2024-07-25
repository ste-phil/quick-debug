<script lang="ts">
  import DeviceMananger from "@comps/DeviceManager.svelte";
  import MessageManager from "@comps/MessageManager.svelte";
  import Chart from "@comps/Charts.svelte";
  import ContextMenu from "@comps/ContextMenu.svelte";
  import {
    freezePlotting,
    darkMode,
    plottingInterval,
    recordingManager,
  } from "@ents/Store";
  import { onMount } from "svelte";
  import PlotModal from "@comps/PlotModal.svelte";
  import RecordingModal from "@comps/RecordingModal.svelte";

  $: isRecording = recordingManager.isRecording;

  function onKeyPressed(e: KeyboardEvent) {
    if (e.key === " ") {
      freezePlotting.set(!$freezePlotting);
    }
  }

  function changeColorMode() {
    if ($darkMode) {
      document.body.classList.add("dark");
    } else {
      document.body.classList.remove("dark");
    }
  }

  function onChangeColorMode() {
    darkMode.set(!$darkMode);

    changeColorMode();
  }

  onMount(() => {
    // recordingManager.endRecording();
    changeColorMode();
  });
</script>

<svelte:window on:keypress={onKeyPressed} />
<nav class="left drawer min no-margin no-padding">
  <div class="surface-container-low row vertical max responsive">
    <DeviceMananger></DeviceMananger>
    <div class="max"></div>
    <MessageManager></MessageManager>
  </div>
</nav>
<main class="responsive max">
  <Chart></Chart>
  <PlotModal></PlotModal>
  <RecordingModal></RecordingModal>
</main>
<footer class="fixed surface-container" style="min-block-size: 3rem">
  <nav>
    <!-- <div class="large-space"></div> -->
    <button data-ui="#chart-plotting-modal" class="circle transparent">
      <i>bar_chart</i>
    </button>
    <button data-ui="#recording-modal" class="circle transparent">
      <i class:videocam-active={$isRecording}>videocam</i>
    </button>
    <div class="max"></div>

    <button class="circle transparent" on:click={onChangeColorMode}>
      <i>{$darkMode ? "light_mode" : "dark_mode"}</i>
    </button>
    <div class="field label small-round border small" style="min-width: 200px;">
      <select bind:value={$plottingInterval}>
        <option id="200">200</option>
        <option id="400">400</option>
        <option id="600">600</option>
      </select>
      <label>Points to display</label>
      <i>arrow_drop_down</i>
    </div>

    <button
      class="circle transparent"
      on:click={() => {
        freezePlotting.set(!$freezePlotting);
      }}
    >
      <i>{$freezePlotting ? "play_arrow" : "pause"}</i>
    </button>
  </nav>
</footer>
<ContextMenu></ContextMenu>

<style>
  .videocam-active::after {
    content: "";
    position: absolute;
    width: 0.35rem;
    height: 0.35rem;
    left: 8px;
    background-color: red;
    border-radius: 50%;
    animation: blink 1s infinite;
  }

  @keyframes blink {
    0%,
    100% {
      opacity: 1;
    }
    50% {
      opacity: 0;
    }
  }
</style>
