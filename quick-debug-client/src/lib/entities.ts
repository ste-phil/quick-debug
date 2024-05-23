import { writable, type Writable } from 'svelte/store';

export class IpData {
    public Socket: WebSocket | null = null;
    
    constructor(
        public IpAddress: string, 
        public IsEnabled: boolean,
        public ConnectionState: ConnectionState) 
    { };
}

export enum ConnectionState {
    Disconnected = 0,
    Connecting = 1,
    Connected = 2
}

export class ChartContext {
    public WasmContext: any = null;
    public ChartSurface: any = null;

    constructor(wasmContext: any, chartSurface: any) {
        this.WasmContext = wasmContext;
        this.ChartSurface = chartSurface;
    }
}

export abstract class Settings {
    public static Port: number = 8126;
}
