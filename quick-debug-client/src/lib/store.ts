import { writable, type Writable } from "svelte/store";
import { ChartContext, ConnectionState, IpData } from "./entities";
import type { XyDataSeries } from "scichart";

export const trackedIps: Writable<IpData[]> = ClearConnectionState(UseLocalStorage("tracked-ips", [
    new IpData("192.168.1.2", true, ConnectionState.Disconnected),
    new IpData("192.168.3.123", true, ConnectionState.Connecting	),
    new IpData("2001:0:130F::09C0:876A:130B", false, ConnectionState.Connected),
]));

export const freezePlotting = writable(false);
export const messageStore: Writable<Map<string, XyDataSeries>> = writable(new Map());
export const chartContext: Writable<ChartContext | null> = writable(null);

function ClearConnectionState(store: Writable<IpData[]> ) {
    store.update(x => {
        x.forEach(ip => ip.ConnectionState = ConnectionState.Disconnected);
        return x;
    });

    return store;
}

function UseLocalStorage(key: string, initial: IpData[]) {
    const value = localStorage.getItem(key)
    const store = writable(value == null ? initial : JSON.parse(value));
    store.subscribe(v => localStorage.setItem(key, JSON.stringify(v)));
    
    return store;
}
