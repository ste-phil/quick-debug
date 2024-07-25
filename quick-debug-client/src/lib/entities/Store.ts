import { writable, type Writable } from "svelte/store";
import { IpDataStore } from "./IpDataStore";
import { ChartManager } from "./ChartManager";
import { UseLocalStorage } from "./Utils";
import { ConfigMessage } from "./Entities";
import { RecordingManager } from "./RecordingManager";


export const chartManager = new ChartManager();
export const recordingManager = new RecordingManager();
export const ipDataStore: IpDataStore = new IpDataStore()
export const configMessages: Writable<ConfigMessage[]> = UseLocalStorage("config-messages", [
	new ConfigMessage("TestMessage", "8000"),
]);
export const configKeys: Writable<string[]> = writable([]);

export const freezePlotting = writable(false);
export const darkMode = UseLocalStorage("dark-mode", true);
export const plottingInterval = UseLocalStorage("plotting-interval", 400);