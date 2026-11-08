## Build 20 changes

Build 20 builds on build 19. Its main change is that **Ocarina of Time's texture flickering is gone** in testing, without the frame-fence wait that cost over 8 ms a frame. It also adds several CPU-side performance changes.

### Ocarina of Time: texture flickering fixed

- **Occlusion counter:** Before each Vulkan submission the emulator reset the ZPass pixel counter. When a submission landed between the game's own counter reset and its report, the report read too low and showed as one-frame white flashes, often on Link. The counter is now paused instead of reset, and a running query is bounded so title screens that never reset it cannot grow it without limit (ported from Eglantine nx2-experimental, Kaj, #121).
- **Native uniform banks:** Native programs keep per-draw tables in 64 KiB uniform banks, and some writes there go unnoticed by the memory tracker, so a material could draw with frame-old data and flash white (tunic, leaves). Each bank is now checked once per frame: every 4 KiB page is hashed, and only the pages whose bytes changed since the host copy was last updated are uploaded again. `EDEN_NX2_BANK_REUPLOAD=0` turns the check off; `EDEN_NX2_BANK_HASH=0` re-uploads whole banks instead of changed pages (re-upload ported from Eglantine nx2-experimental; page hashing is new in this build).
- **Shader lowering:** Native programs that were rejected with "native program uses lowering scratch registers" (a misaligned high uniform base) or "native register shadowing mode" (the register form of SHF) now lower, so their draws are no longer skipped (ported from Lichii, Frysix, #241).

In testing the flickering no longer appeared in Ocarina of Time.

### Performance

- **Draw setup and memory:** GPU-written pages are marked a bitmap word at a time (zortos293), draws reuse the graphics pipeline they already selected for their dynamic state (Frizti), and GPU-to-device address translations are cached per thread until the page table changes.
- **Render targets:** A new render target that a clear overwrites whole is no longer uploaded from guest memory first, and the check now matches the clear's real coverage (surface clip, flipped scissor, mapped target) (ported from Eglantine nx2-experimental and Lichii, Kaj, #141 and #311).
- **Suspended channels:** A channel waiting on a semaphore the CPU releases is polled every 50 µs instead of every 1 ms during its first 2 s of waiting, with the GPU thread's timer slack lowered on Linux (ported from Lichii, ne1i, #446).
- **Quad index buffer:** Growing it no longer waits for the GPU to finish (ported from Lichii, Zortos, #414).

Measured by the tester on Linux in Ocarina of Time:

- The draw setup and memory changes: about +2–3 FPS in Hyrule Market and Cocorico Village.
- The flicker fixes cost frame time at first (Cocorico Village test spot down to about 40 FPS). Re-uploading only the changed bank pages brought it back to about 43 FPS, with no flickering.

### Tested

- Ocarina of Time: no texture flickering in testing.
- Splatoon Raiders and Mario Kart World: no regressions found.

### Not included

- "Wait for each of Ocarina of Time's frames" (`a23be213d5`) is still not part of this build: it costs over 8 ms a frame and does not fully fix the flickering.

Thanks to Kaj, Frysix, ne1i, Zortos, zortos293, Frizti, Duck912 and the Lichii and Eglantine contributors for the ported fixes, and to Jojo for the Ocarina of Time work this build continues from.
