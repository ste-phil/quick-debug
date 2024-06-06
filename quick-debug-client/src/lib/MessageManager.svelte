<script lang="ts">
    import { ConfigMessage } from "./entities";
    import { configMessages, ipDataStore } from "./store";


    function addMessage(event: MouseEvent & { currentTarget: EventTarget & HTMLButtonElement; }) {
        configMessages.set([...$configMessages, new ConfigMessage("", "")])
    }

    function sendMessage(i: number) {
        ipDataStore.SendMessageToAll($configMessages[i])
    }
</script>

<div class="responsive no-padding no-space ">
    {#each $configMessages as msg, i}
        <div class="row max responsive no-space center-align margin-left margin-right context-menu">
            <div class="field label border no-padding no-margin small left-round short small-round">
                <input type="text" id="input-ip" bind:value={msg.Key}/>
                <label for="input-ip">Key</label>
            </div>
            <div class="field label border no-padding no-margin small no-round short">
                <input type="text" id="input-ip" bind:value={msg.Value}/>
                <label for="input-ip">Value</label>
            </div>
            <button class="right-round short-btn small-round no-padding" on:click={() => sendMessage(i)}>
                <i>play_arrow</i>
            </button>
        </div>
        
    {/each}

    <button class="responsive small-round margin-top " on:click={addMessage}>
        <i>add</i>
        <span>Button</span>
    </button>
</div>

<style>
    .short {
        width: 125px;
    }

    .short-btn {
        width: 35px;
        min-width: 35px;
    }
</style>