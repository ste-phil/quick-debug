import { writable } from "svelte/store";

export function UseLocalStorage<T>(key: string, initial: T) {
	const value = localStorage.getItem(key);
	let parsedValue: T;

	try {
		parsedValue = value ? JSON.parse(value) as T : initial;
	} catch (e) {
		parsedValue = initial;
	}

	const store = writable(parsedValue);
	store.subscribe(v => localStorage.setItem(key, JSON.stringify(v)));

	return store;
}

export function UseLocalStorageMap<K, V>(key: string, initial: Map<K, V>) {
	const value = localStorage.getItem(key);
	let parsedValue: Map<K, V>;

	try {
		parsedValue = value ? new Map(JSON.parse(value)) as Map<K, V> : initial;
	} catch (e) {
		parsedValue = initial;
	}

	const store = writable(parsedValue);
	store.subscribe(v => localStorage.setItem(key, JSON.stringify(Array.from(v.entries()))));

	return store;
}


export function ReadWritable<T>(store: { subscribe: (run: (value: T) => void) => () => void }): T {
	let value: T;
	const unsubscribe = store.subscribe(v => {
		value = v
	});
	unsubscribe();
	return value;
}