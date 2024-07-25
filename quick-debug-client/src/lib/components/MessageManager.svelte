<script lang="ts">
  import { ConfigMessage } from "@ents/Entities";
  import { configKeys, configMessages, ipDataStore } from "@ents/Store";

  function addMessage(
    event: MouseEvent & { currentTarget: EventTarget & HTMLButtonElement }
  ) {
    configMessages.set([...$configMessages, new ConfigMessage("", "")]);
  }

  function sendMessage(i: number) {
    ipDataStore.SendMessageToAll($configMessages[i]);
  }
</script>

<datalist id="input-msg-key-dl">
  {#each $configKeys as key}
    <option>{key}</option>
  {/each}
</datalist>

<div id="override-padding" class="responsive no-padding no-space">
  {#each $configMessages as msg, i}
    <div
      class="row max responsive no-space center-align no-padding context-menu"
    >
      <div
        class="field label border no-padding no-margin small left-round short small-round"
      >
        <input
          type="text"
          id="input-msg-key"
          bind:value={msg.Key}
          list="input-msg-key-dl"
        />
        <label for="input-msg-key">Key</label>
      </div>
      <div class="field label border no-padding no-margin small no-round short">
        <input type="text" id="input-msg-value" bind:value={msg.Value} />
        <label for="input-msg-value">Value</label>
      </div>
      <button
        class="right-round short-btn small-round no-padding"
        on:click={() => sendMessage(i)}
      >
        <i>play_arrow</i>
      </button>
    </div>
  {/each}

  <button
    id="margin-custom"
    class="responsive small-round margin-custom"
    on:click={addMessage}
  >
    <i>add</i>
    <span>Add</span>
  </button>
</div>

<style>
  #override-padding {
    padding: 0 8px 0 8px !important;
  }

  #margin-custom {
    margin: 16px 0 8px 0 !important;
  }

  .short {
    width: 135px;
  }

  .short-btn {
    width: 35px;
    min-width: 35px;
  }
</style>
