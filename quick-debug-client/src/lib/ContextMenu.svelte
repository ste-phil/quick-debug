<script lang="ts">
    import { ipDataStore } from "./store";

    // pos is cursor position when right click occur
    let pos = { x: 0, y: 0 }
    // menu is dimension (height and width) of context menu
    let menu = { h: 0, y: 0 }
    // browser/window dimension (height and width)
    let browser = { h: 0, y: 0 }
    // showMenu is state of context-menu visibility
    let showMenu = false;
    let target: HTMLElement | null = null;

    export class MenuItem {
        displayText: string;
        icon: string;
        onClick: () => void;

        constructor(displayText: string, icon: string, onClick: () => void) {
            this.icon = icon;
            this.displayText = displayText;
            this.onClick = onClick;
        }
    }

    function onContextMenu(e: MouseEvent){
        target = e.target as HTMLElement;
        //check if any parent of the target has "context-menu" class
        if (!target.closest(".context-menu")) {
            return;
        }

        e.preventDefault();
        showMenu = true
        browser = {
            w: window.innerWidth,
            h: window.innerHeight
        };
        pos = {
            x: e.clientX,
            y: e.clientY
        };

        
        // If bottom part of context menu will be displayed
        // after right-click, then change the position of the
        // context menu. This position is controlled by `top` and `left`
        // at inline style. 
        // Instead of context menu is displayed from top left of cursor position
        // when right-click occur, it will be displayed from bottom left.
        if (browser.h -  pos.y < menu.h)
            pos.y = pos.y - menu.h
        if (browser.w -  pos.x < menu.w)
            pos.x = pos.x - menu.w
    }
    function onPageClick(e: any){
        // To make context menu disappear when
        // mouse is clicked outside context menu
        showMenu = false;
    }
    function getContextMenuDimension(node: any){
        // This function will get context menu dimension
        // when navigation is shown => showMenu = true
        let height = node.offsetHeight
        let width = node.offsetWidth
        menu = {
            h: height,
            w: width
        }
    }
    let menuItems: MenuItem[] = [
        new MenuItem("Remove", "delete", () => {
            const elem = target.closest(".context-menu");
            //find first child element matching the id "ip-address"
            //and remove it from trackedIps
            const ip = elem?.querySelector("#ip-address")?.textContent;
            if (ip) {
                ipDataStore.removeIp(ip);
            }
        }),
    ]

</script>
<svelte:head>
    <!-- You can change icon sets according to your taste. Change `class` value in `menuItems` above to represent your icons. -->
    <!-- <link rel="stylesheet" href="/icon/css/mfglabs_iconset.css"> -->
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.1.1/css/all.min.css" integrity="sha512-KfkfwYDsLkIlwQp6LFnl8zNdLGxu9YAA1QvwINks4PhcElQSvqcyVLLD9aMhXd13uQjoXtEKNosOWaZqXgel0g==" crossorigin="anonymous" referrerpolicy="no-referrer" />
</svelte:head>
<style>
    * {
        padding: 0;
        margin: 0;
        z-index: 3000;
    }
    #navbar{
        display: inline-flex;
        background-color: transparent;
        overflow: hidden;
        flex-direction: column;
    }
</style>

{#if showMenu}
<nav use:getContextMenuDimension style="position: absolute; top:{pos.y}px; left:{pos.x}px">
    <div id="navbar">
        <ul class="responsive">
            {#each menuItems as item}
                <button on:click={item.onClick} class="small-blur border transparent small-round" style="padding: 4px 16px 4px 16px;">
                    <i>{item.icon}</i>
                    <span style="width: 100px; text-align: right;">{item.displayText}</span>
                </button>
            {/each}
        </ul>
    </div>
</nav>
{/if}

<svelte:window 
    on:contextmenu={onContextMenu} 
    on:click={onPageClick} 
/>

