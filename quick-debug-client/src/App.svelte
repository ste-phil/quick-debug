<script lang="ts">
  import DeviceMananger from './lib/DeviceManager.svelte'
  import MessageManager from './lib/MessageManager.svelte';
  import Chart from './lib/Chart.svelte'
  import ContextMenu from './lib/ContextMenu.svelte';
  import { freezePlotting, darkMode } from './lib/store';
  import { onMount } from 'svelte';

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

    changeColorMode()
  }

  onMount(() => {
    changeColorMode()
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
<main class="responsive">
  <Chart></Chart>
</main>
<footer style="min-block-size: 3rem;">
  <nav>
    <div class="large-space"></div>

    <div class="row no-padding no-space small-margin">
      <div class="field label left-round border small" style="min-width: 200px;">
        <select>
          <option>Item 1</option>
          <option>Item 2</option>
          <option>Item 3</option>
        </select>
        <label>Recording</label>
        <i>arrow_drop_down</i>
      </div>
      <button class="small-round no-padding no-margin right-round small-round">
        <i>add</i>
      </button>

      <button class="circle transparent">
        <i>download</i>
      </button>
    </div>


    <div class="max"></div>
    <button class="circle transparent" on:click={onChangeColorMode}>
      <i>{$darkMode ? "light_mode" : "dark_mode"}</i>
    </button>
    <button class="circle transparent" on:click={() => {freezePlotting.set(!$freezePlotting)}}>
      <i>{$freezePlotting ? "play_arrow" : "pause"}</i>
    </button>
  </nav>
</footer>
<ContextMenu></ContextMenu>


<style>
</style>
