// SPDX-License-Identifier: GPL-3.0-or-later

#include <array>
#include <thread>
#include <vector>
#include <catch2/catch_test_macros.hpp>

#include "video_core/command_fetch_order.h"
#include "video_core/nx2.h"

TEST_CASE("First queued GPU command write is visible before snapshot selection",
          "[video_core][command_fetch_order]") {
    constexpr u64 address = 0x8000;
    Tegra::Nx2ClearGpuWrittenPages();
    const std::array<u32, 5> snapshot{0, 1, 0, 0, 0};
    std::array<u32, 5> host = snapshot;
    std::array<u32, 5> gpu = snapshot;
    std::vector<int> events;
    bool dirty = false;
    REQUIRE_FALSE(Tegra::Nx2IsGpuWritten(address));
    const bool written = VideoCommon::SynchronizeCommandFetch(
        [&] {
            events.push_back(1);
            gpu = {36, 1, 12, 0, 0};
            dirty = true;
            Tegra::Nx2MarkGpuWritten(address, sizeof(gpu));
        },
        [&] {
            events.push_back(2);
            return Tegra::Nx2IsGpuWritten(address);
        },
        [&] {
            events.push_back(3);
            REQUIRE(dirty);
            host = gpu;
            dirty = false;
        });
    REQUIRE(events == std::vector<int>{1, 2, 3});
    REQUIRE(written);
    REQUIRE_FALSE(dirty);
    const bool use_snapshot = VideoCommon::CanUseSubmittedCommands(false, true, written, dirty);
    REQUIRE_FALSE(use_snapshot);
    REQUIRE((use_snapshot ? snapshot : host) == gpu);
    REQUIRE(host != snapshot);
    Tegra::Nx2ClearGpuWrittenPages();
}

TEST_CASE("Known GPU-written pages never revert to a stale snapshot after flushing",
          "[video_core][command_fetch_order]") {
    constexpr u64 address = 0x9000;
    Tegra::Nx2ClearGpuWrittenPages();
    Tegra::Nx2MarkGpuWritten(address, sizeof(u32));
    u32 host = 0;
    u32 gpu = 0;
    u32 drains = 0;
    u32 flushes = 0;
    for (u32 frame = 1; frame <= 128; ++frame) {
        const bool written = VideoCommon::SynchronizeCommandFetch(
            [&] { ++drains; gpu = frame; },
            [&] { return Tegra::Nx2IsGpuWritten(address); },
            [&] { ++flushes; host = gpu; });
        REQUIRE(written);
        REQUIRE_FALSE(VideoCommon::CanUseSubmittedCommands(false, true, written, false));
        REQUIRE(host == frame);
    }
    REQUIRE(drains == 128);
    REQUIRE(flushes == 128);
    Tegra::Nx2ClearGpuWrittenPages();
}

TEST_CASE("Clean command pages still drain before detection but need no flush",
          "[video_core][command_fetch_order]") {
    bool drained = false;
    const bool written = VideoCommon::SynchronizeCommandFetch(
        [&] { drained = true; },
        [&] { REQUIRE(drained); return false; },
        [] { FAIL("Clean commands must not be downloaded"); });
    REQUIRE_FALSE(written);
    REQUIRE(VideoCommon::CanUseSubmittedCommands(false, true, written, false));
}

TEST_CASE("Command snapshots require fresh non-resumed CPU-only entries",
          "[video_core][command_fetch_order]") {
    for (unsigned bits = 0; bits < 16; ++bits) {
        const bool resumed = (bits & 1) != 0;
        const bool snapshot = (bits & 2) != 0;
        const bool written = (bits & 4) != 0;
        const bool dirty = (bits & 8) != 0;
        REQUIRE(VideoCommon::CanUseSubmittedCommands(resumed, snapshot, written, dirty) ==
                (bits == 2));
    }
}

TEST_CASE("Synchronization failure cannot fall through to stale command words",
          "[video_core][command_fetch_order]") {
    bool checked = false;
    bool flushed = false;
    REQUIRE_THROWS_AS(VideoCommon::SynchronizeCommandFetch(
        [] { throw 42; },
        [&] { checked = true; return true; },
        [&] { flushed = true; }), int);
    REQUIRE_FALSE(checked);
    REQUIRE_FALSE(flushed);
}

TEST_CASE("GPU-written page marks cover word and address-space boundaries",
          "[video_core][command_fetch_order]") {
    constexpr u64 page_size = 4096;
    Tegra::Nx2ClearGpuWrittenPages();
    Tegra::Nx2MarkGpuWritten(63 * page_size + 100, 66 * page_size);
    REQUIRE_FALSE(Tegra::Nx2IsGpuWritten(62 * page_size));
    for (u64 page = 63; page <= 129; ++page) {
        CAPTURE(page);
        REQUIRE(Tegra::Nx2IsGpuWritten(page * page_size));
    }
    REQUIRE_FALSE(Tegra::Nx2IsGpuWritten(130 * page_size));
    constexpr u64 last_page = (u64{1} << 36) - page_size;
    Tegra::Nx2MarkGpuWritten(last_page, 2 * page_size);
    REQUIRE(Tegra::Nx2IsGpuWritten(last_page));
    REQUIRE_FALSE(Tegra::Nx2IsGpuWritten(u64{1} << 36));
    Tegra::Nx2ClearGpuWrittenPages();
}

TEST_CASE("Concurrent GPU-written page marks retain every range", "[video_core][command_fetch_order]") {
    constexpr u64 page_size = 4096;
    constexpr u64 page_count = 512;
    Tegra::Nx2ClearGpuWrittenPages();
    std::array<std::thread, 8> writers;
    for (u64 writer = 0; writer < writers.size(); ++writer) {
        writers[writer] = std::thread([writer] {
            for (u64 i = 0; i < 1000; ++i) {
                const u64 page = (i * 37 + writer * 101) % page_count;
                Tegra::Nx2MarkGpuWritten(page * page_size, page_size);
            }
        });
    }
    for (auto& writer : writers) {
        writer.join();
    }
    for (u64 page = 0; page < page_count; ++page) {
        CAPTURE(page);
        REQUIRE(Tegra::Nx2IsGpuWritten(page * page_size));
    }
    Tegra::Nx2ClearGpuWrittenPages();
}
