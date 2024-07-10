import { writable, type Invalidator, type Subscriber, type Unsubscriber, type Updater, type Writable } from "svelte/store";
import { IpDataStore } from "./IpDataStore";
import { ChartContext, ChartManager } from "./ChartManager";
import { UseLocalStorage } from "./Utils";
import { ConfigMessage } from "./Entities";


export const chartManager: ChartManager = new ChartManager();
export const ipDataStore: IpDataStore = new IpDataStore()
export const configMessages: Writable<ConfigMessage[]> = UseLocalStorage("config-messages", [
	new ConfigMessage("TestMessage", "8000"),
]);
export const configKeys: Writable<string[]> = writable([]);

export const freezePlotting = writable(false);
export const darkMode = UseLocalStorage("dark-mode", true);
export const plottingInterval = UseLocalStorage("plotting-interval", 100);

export const messageStore: Writable<Map<string, XyDataSeries>> = writable(new Map());
export const chartContext: Writable<ChartContext | null> = writable(null);