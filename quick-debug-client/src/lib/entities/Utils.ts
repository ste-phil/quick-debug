import { writable } from "svelte/store";

export function UseLocalStorage<T>(key: string, initial: T) {
	const value = localStorage.getItem(key)
	const store = writable(value == null ? initial : JSON.parse(value));
	store.subscribe(v => localStorage.setItem(key, JSON.stringify(v)));

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