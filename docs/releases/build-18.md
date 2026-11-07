## Build 18 changes

- **Splatoon Raiders:** Run GPU macros on the split back thread for Raiders. This restores squid-form interaction with ink in the tested offshore platform scene while keeping the split GPU thread enabled. The exact front-thread macro defect is still being investigated.
- **GPU timing:** Restore the effect of Boost and Overclock on Switch 2 GPU timestamps for Raiders and Ocarina of Time. In the user's Raiders test, the offshore platform shimmer disappeared and performance returned. Ocarina of Time still needs separate verification.
- **Ocarina of Time:** Add shader and buffer-pointer changes for vegetation, grass, doors and vines, plus a fix for the sky-ambient buffer binding. These are intended to address missing vegetation and incorrect material lighting; results can vary by scene and game version.
- **Rendering and caches:** Keep native draws needed for persistent textures during asynchronous shader builds, retain used Vulkan pipelines and native textures, and cap the native texture cache at up to 10 GiB where the available budget permits. Shader cache version changes can require shaders to be rebuilt once.
- **Compatibility:** Build on current Eglantine `nx2` and integrate Ocarina support from [Jojo's Eglantine fork](https://github.com/jojo-chaechae/Eglantine), with selected Lichii changes. The changes from `9fea7d8379`, `b13c4c1c0c` and `15126fbdc1` remain reverted.

## Known issues

- Ocarina of Time can still look pixelated or have poor edge quality, including at 1x docked resolution. The timestamp change has not yet been confirmed to resolve this.
- Texture flickering can still occur in Ocarina of Time, especially on foliage and the ground.
- Ocarina of Time fog and lighting can remain excessive in some scenes; the underlying cause is still being investigated.

Thanks to Jojo and the Lichii contributors, including Zarigis, Duck912 and zortos293, for the upstream compatibility and rendering work.
