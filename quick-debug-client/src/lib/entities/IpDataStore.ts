import type { Writable } from "svelte/store";
import { UseLocalStorage } from "./Utils";

export enum ConnectionState {
	Disconnected = 0,
	Connecting = 1,
	Connected = 2
}

export class IpData {
	public Socket: WebSocket | null = null;

	constructor(
		public IpAddress: string,
		public IsEnabled: boolean,
		public ConnectionState: ConnectionState) { };
}

export class IpDataStore {
	private readonly trackedIps: Writable<IpData[]>;

	constructor() {
		this.trackedIps = this.ClearConnectionState(UseLocalStorage("tracked-ips", [
			new IpData("127.0.0.1", true, ConnectionState.Disconnected),
			new IpData("::1", true, ConnectionState.Disconnected),
		]));
	}

	private ClearConnectionState(store: Writable<IpData[]>) {
		store.update(x => {
			x.forEach(ip => ip.ConnectionState = ConnectionState.Disconnected);
			return x;
		});

		return store;
	}

	[Symbol.iterator]() {
		let index = 0;
		let values: IpData[] = [];
		this.subscribe((value) => {
			values = Array.isArray(value) ? value : [];
		})();

		return {
			next: () => {
				if (index < values.length) {
					return { value: values[index++], done: false };
				} else {
					index = 0;
					return { done: true };
				}
			}
		};
	}

	public addIp(data: IpData) {
		this.trackedIps.update(x => [...x, data]);
	}

	public removeIp(ip: string) {
		this.trackedIps.update(ips => {
			let elemToRemove = ips.find(x => x.IpAddress === ip)
			if (elemToRemove != undefined) {
				ips.splice(ips.indexOf(elemToRemove), 1);
				elemToRemove.Socket?.close();
				return ips;
			}
			return ips;
		});
	}

	public subscribe(val: (value: any) => void) {
		return this.trackedIps.subscribe(val);
	}

	public update() {
		this.trackedIps.update(x => x);
	}

	public Get(): Writable<IpData[]> {
		return this.trackedIps;
	}

	public SendMessageToAll(msg: ConfigMessage) {
		for (const e of this) {
			if (e?.Socket?.send !== undefined)
				e?.Socket?.send(`${msg.Key};${msg.Value}`)
		}
	}
}
