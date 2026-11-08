## Build 21 changes

Build 21 builds on build 20 (branch `nx2-opt`). Its main changes are **faster Ocarina of Time gameplay**, mostly in heavy areas, and **near bushes, hedges and vines that keep their leaves** up close.

### Performance

- **GPU thread split:** Small command entries no longer wait for all queued compute and copy work before they are read; only Hyrule Warriors: Age of Imprisonment keeps that wait. In Ocarina of Time it took about 75% of the GPU thread's front half. `EDEN_NX2_SMALL_ENTRY_WAIT=1` brings the wait back for any game.
- **Matrix multiplies:** Ocarina of Time's tensor (HMMA) work runs on the GPU's cooperative matrices instead of being emulated, after a self-test at startup (ported from Lichii, Zortos and Marc Stampfli, #582).
- **Texture uploads:** Block-linear textures whose rows are whole GOBs are unswizzled a GOB at a time (1.5–1.7x faster with the standard tiling, checked byte for byte against the previous code), and a new image whose guest memory is all zero is cleared on the GPU instead of uploaded (`EDEN_NX2_ZERO_UPLOAD=0` turns that off).
- **Draw setup:** Storage buffer bindings are cached until the page table changes (Matt Artist), instance-loop writes the back half ignores are no longer sent, cache invalidations skip caches with nothing on the written pages, flush checks skip the buffer cache lock for pages the GPU never wrote, and several smaller per-draw and per-fence costs are gone.
- **Pipelines:** Cached pipelines that boot skips are built on background threads (`EDEN_NX2_BACKGROUND_PIPELINES=0` turns it off), shader helpers for native memory access stay function calls, which cuts NVIDIA's compile time for Ocarina's foliage programs from about 2.4 s to 0.6 s each (`EDEN_NX2_LEAN_SSBO=0` restores the old code), and alpha-test fades no longer build a pipeline per frame.
- **Staging memory:** Idle upload staging buffers up to 32 MiB are kept instead of freed every 64 frames.

### Ocarina of Time: near foliage

Bushes and hedges near the Temple of Time and Hyrule Castle lost their leaves up close. Several causes are fixed:

- Mesh programs that output points or lines now compile (Frizti).
- The hedge culling compute program is no longer rejected for its call linkage: a call whose return-address move the compiler scheduled a few instructions earlier, and the P2R instruction between them, are now understood.
- Foliage placement records read through a loaded pointer are bound to the right buffer (Duck912).
- A compute pipeline translated while one of its pointer slots was null is rebuilt once the slot holds a buffer, including a build loaded from the disk shader cache (Duck912).

### Diagnostics

With `EDEN_NX2_GPU_PROFILE=1` the GPU thread reports also show host GPU waits, contended buffer and texture cache lock waits, unknown guest writes and how MME DMA reads wait. With `HW_GPU` debug logging, the texture cache lists the most uploaded image shapes.

### Not included

- The early semaphore acquire check is in the code but off by default: it drew a light shaft as a white block and wall objects black in Ocarina of Time. `EDEN_NX2_ACQUIRE_EARLY=1` turns it on for testing only.
- "Wait for each of Ocarina of Time's frames" (`a23be213d5`) is still not part of this build.

### Tested

- Ocarina of Time on Linux (RTX 5070): better FPS in heavy areas, near bushes, hedges and vines keep their leaves, no lighting issues.

## Credits

These changes were taken from other forks; each keeps its original author in the commit history, except where noted as ported by hand.

| Change | Author | Source | Commit in this build |
| --- | --- | --- | --- |
| Fence operation lists as vectors | Ale | Eglantine nx2-experimental `d52e5f1036` | `d04719d548` |
| Usage masks for empty and partial blocks | zortos293 | Eglantine nx2-experimental `5778922694` | `ca005017e5` |
| Round the alpha test reference to 1/255 | zortos293 | Eglantine nx2-experimental `3a4c00a3c9` (#544) | `5458196939` |
| Cache storage buffer bindings per page table serial | Matt Artist | Eglantine nx2-experimental `c8415d0e80` | `7a57d9a5b4` |
| Build skipped cached pipelines on background threads | Frizti | Eglantine nx2-experimental `72f3c47996` | `e4f5acb457` |
| Keep native global memory helpers as calls | Duck912 | Eglantine nx2-experimental `9a362357a2` | `b1a48b6559` |
| HMMA on cooperative matrices | Zortos, Marc Stampfli | Lichii `8318201fa2` (#582, #475) | `aa1ac0c5c7` |
| Skip instance loop writes the back ignores | Thibaut Léaux, Marc Stampfli | Lichii `7e2a9fe20c` (#178), `5ba52f7720` (#488) | `441d6149c3` (port) |
| Skip invalidations of caches with nothing on the pages | Thibaut Léaux | Lichii `e67d389592` (#250) | `d1609b0506` |
| Invalidate every block a cached GPU write touches | Zortos | Lichii `821ccd570a` (#700) | `7a3f7077f3` |
| MustFlushRegion without the buffer cache lock | Frizti, Zortos | Lichii `2b70fc5ee9` (#413) | `3e53c3cd33` (port) |
| Reuse a contiguous range's page table walk | Marc Stampfli | Lichii `7083185c43` (#580, #478) | `4a9e2bc5f3` (port) |
| Keep idle upload staging buffers | Frizti | Lichii `794602bfbd` (#778) | `a46089c23f` (port) |
| Read EDEN_NX2_SKIP_SAME once per engine | Marc Stampfli | Lichii `794602bfbd` (#778, #488) | `251d09e321` (port) |
| Mesh programs that output points or lines | Frizti | Eglantine nx2-experimental `743d5bbaed` | `f451c44ef8` |
| Bind loaded pointers that compete with a paired cbuf pointer | Duck912 | Eglantine nx2-experimental `a51a69fbfb` (#715) | `93588f44c0` |
| Rebuild compute pipelines once a null pointer slot holds a buffer | Duck912 | Eglantine nx2-experimental `688c6fa7d8` | `a207ff25e3` |

P2R as a known register write (`94ed300d94`) matches Lichii's `f356d39ba3` (Kradnah, #144).

Written for this fork: the Hyrule-Warriors-only small entry wait (`8d73beb618`), GOB-at-a-time unswizzling (`1f70d91b24`), zero-memory image clears (`c6ec054e46`, `ceccecb37a`), the scheduled call linkage search (`4ceb43a787`), the framebuffer walk skip (`fc837aa945`), the native bank and uniform binding checks (`12469b4246`, `a44816b316`), the fence allocation fix (`5ae3eaa2d7`) and the profiling additions.

Thanks to everyone above, to the Lichii and Eglantine contributors, and to Jojo for the Ocarina of Time work this build continues from.
