<script lang="ts">
  import { createEventDispatcher } from "svelte";

  const dispatch = createEventDispatcher();
  export let input: string = "";
  export let classes: string = "";

  let errorMessage: string = "";
  export function setError(message: string) {
    errorMessage = message;
  }

  export function clearError() {
    errorMessage = "";
  }

  function handleKeyDown(event: KeyboardEvent) {
    if (event.key === "Enter") {
      dispatch("submit", input);
    }
  }
</script>

<div class="field label prefix {classes} {errorMessage == '' ? '' : 'invalid'}">
  <i>search</i>
  <input
    type="text"
    id="input-ip"
    bind:value={input}
    on:focus={() => clearError()}
    on:keydown={handleKeyDown}
  />
  <label for="input-ip">IP Address</label>
  {#if errorMessage != ""}
    <span class="error hidden">{errorMessage}</span>
  {/if}
</div>

<style>
  .short {
    width: 145px;
  }
</style>
