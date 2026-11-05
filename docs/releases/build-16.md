
## Build 16 changelog

### Splatoon Raiders

- Fixed native tiled draws being skipped while an asynchronous graphics pipeline compiled. This restores persistent texture updates, including inkable surfaces.
- Corrected two Ampere shader lowering cases that could produce incorrect components in effects such as the skybox and lighting.

### Mario Kart World

- Dynamic FPS pacing now resolves the registered application process, so the swap interval follows the game process even when video-interface work runs in a service context.
- Removed the separate fixed 30 FPS mode. The dynamic FPS setting now applies its complete frame-time patch as one unit.
- Reduced split GPU-thread overhead by batching fence and command-list submissions, avoiding unnecessary writer waits, and preserving deferred submission correctness during shutdown.
- Added an opt-in split register-write batch path. Set `EDEN_NX2_SPLIT_BATCH_METHODS=1` to enable it; it is disabled by default.

### Rendering and cache behavior

- Mesh and task graphics pipelines now retain the mesh translation state needed for disk-cache reuse.
- Native texture-cache entries remain resident until the device pressure threshold, reducing repeated uploads after revisiting an area.
- Raised the texture-cache critical-memory cap from 5 GiB to 10 GiB.

### Cache migration

- The Vulkan pipeline-cache format changed to version 22. Existing entries are invalidated automatically on first launch.
