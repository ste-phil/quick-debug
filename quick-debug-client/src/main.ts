import { libraryVersion, SciChartSurface } from 'scichart';
import './app.css'    
import App from './App.svelte'
import "beercss";
import "material-dynamic-colors";

SciChartSurface.loadWasmFromCDN();

// SciChartSurface.configure({

//   dataUrl: `https://cdn.jsdelivr.net/npm/scichart@3.4.623${libraryVersion}/_wasm/scichart2d.data`,
//   wasmUrl: `https://cdn.jsdelivr.net/npm/scichart@3.4.623${libraryVersion}/_wasm/scichart2d.wasm`
// });

const app = new App({
  target: document.getElementById('app')!,
})

export default app
