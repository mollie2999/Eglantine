## Build 19 changes

Build 19 is a performance build on top of build 18. It does not change rendering behaviour on purpose: every change below makes the GPU front and back threads, the buffer cache or texture decoding do less redundant work.

### Performance

- **Ocarina of Time, GPU back thread:** `MaxContinuousRange` answers are now cached per thread with 4096 entries instead of 64. Ocarina of Time binds storage buffers behind loaded pointers on most draws and dispatches, which is far more buffers than the old cache held, so about 25,000 lookups a second at the Kokiri Forest spawn walked the page table (44% of the back thread in stack samples). Misses fall to roughly 7,000 a second over shorter ranges. Measured by the author on an MSVC build with the CLI harness, Kokiri Forest spawn, GPU accuracy Fast: 26.3–27.4 ms per frame before, 22.3–23.2 ms after, about +20% fps. Other scenes and games were not measured. The cache hash was also reworked (`9faf60b6d5`) so that entries whose address and size move together no longer land in the same slot.
- **Split GPU thread:** The 3D front thread no longer waits on queued compute, copy or 2D work just because a plain register was written to one of those engines. Only methods that can write guest memory (uploads, launches, QMD signals, report semaphores, macros) now move the "unknown write" marker, so an MME DMA read on the front thread stops stalling behind unrelated back-thread work.
- **Buffer cache:** Binding clean index, vertex and stage buffers skips word-level work when nothing was modified and clearing cannot change CPU tracking (ported from Lichii). Idle buffer usage trackers are no longer reset on every track (ported from sylvieistaken/selevet-eglantine, via Lichii). Page and recent-range clears after each track are unchanged.
- **Draw setup:** `GetMemoryLayoutSize` keeps the last memory layout interval and answers repeated queries inside it without a new `std::map` search. Every map or unmap invalidates it, including splits, merges and extensions of adjacent intervals (ported from Lichii).
- **Texture decoding:** Native texture rows are now unswizzled two at a time, including textures with odd heights. This is a CPU-side speedup for texture uploads and has an added unit test.

### Tests

- Added a randomized page-level oracle for the buffer cache word skipping and for cached writes during buffer upload scans.
- Added a native unswizzle test and an array-oracle check of memory layout sizes over 20,000 randomized remaps.

### Not included

- "Wait for the OoT frame fence to prevent flashing" was tried for this build and reverted before release, so it is not part of build 19. Ocarina of Time frame pacing is unchanged from build 18.

## Known issues

Nothing in build 19 targets these, and they are expected to behave as in build 18:

- Ocarina of Time can still look pixelated or have poor edge quality, including at 1x docked resolution.
- Texture flickering can still occur in Ocarina of Time, especially on foliage and the ground.
- Ocarina of Time fog and lighting can remain excessive in some scenes.

Thanks to Duck912, zortos293 and the Lichii contributors for the ported optimizations, and to Jojo for the Ocarina of Time work this build continues from.
