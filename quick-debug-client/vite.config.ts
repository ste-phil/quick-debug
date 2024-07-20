import { defineConfig } from 'vite'
import { svelte } from '@sveltejs/vite-plugin-svelte'
import { viteSingleFile } from "vite-plugin-singlefile"
import { viteStaticCopy } from 'vite-plugin-static-copy'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [
    svelte(),
    viteSingleFile(),
    //   viteStaticCopy({
    //     targets: [
    //       {
    //         src: 'node_modules/scichart/_wasm/scichart2d.data',
    //         dest: '/'
    //       },
    //       {
    //         src: 'node_modules/scichart/_wasm/scichart2d.wasm',
    //         dest: '/'
    //       },
    //         // same for scichart3d if needed
    //     ]
    // }),
  ],
})
