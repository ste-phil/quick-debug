<script lang="ts">
  import svelteLogo from './assets/svelte.svg'
  import viteLogo from '/vite.svg'
  import Sidebar from './lib/Sidebar.svelte'
  import Chart from './lib/Chart.svelte'
  import ContextMenu from './lib/ContextMenu.svelte';
  import { freezePlotting } from './lib/store';
</script>

<nav class="left drawer min no-margin no-padding">
  <Sidebar></Sidebar>
</nav>
<main class="responsive">
  <Chart></Chart>
</main>
<footer style="min-block-size: 3rem;">
  <nav>
    <div class="max"></div>
    <button class="circle transparent" on:click={() => {freezePlotting.set(!$freezePlotting)}}>
      <i>{$freezePlotting ? "play_arrow" : "pause"}</i>
    </button>
  </nav>
</footer>
<ContextMenu></ContextMenu>

<style>
</style>
