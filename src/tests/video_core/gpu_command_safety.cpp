// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <array>
#include <limits>
#include <utility>
#include <catch2/catch_test_macros.hpp>

#include "core/core.h"
#include "core/device_memory.h"
#include "tests/core/system_test_context.h"
#include "video_core/host1x/syncpoint_manager.h"
#include "video_core/memory_manager.h"
#include "video_core/renderer_null/null_rasterizer.h"

TEST_CASE("GPU command ranges cannot walk past the address space", "[gpu_command_safety]") {
    ScopedSystemTestContext context;
    Core::System system;
    Core::DeviceMemory physical;
    Tegra::MaxwellDeviceMemoryManager device_memory{physical};
    Tegra::MemoryManager memory{system, device_memory, 32, 1ULL << 31};
    constexpr GPUVAddr end = 1ULL << 32;

    for (const auto [address, size] : std::array{
             std::pair<GPUVAddr, size_t>{end, 1},
             std::pair<GPUVAddr, size_t>{end - 1, 2},
             std::pair<GPUVAddr, size_t>{std::numeric_limits<u64>::max(), 2},
             std::pair<GPUVAddr, size_t>{1, std::numeric_limits<size_t>::max()}}) {
        CAPTURE(address, size);
        REQUIRE_FALSE(memory.IsFullyMappedRange(address, size));
        REQUIRE_FALSE(memory.IsGranularRange(address, size));
        REQUIRE_FALSE(memory.IsContinuousRange(address, size));
        REQUIRE(memory.GetSubmappedRange(address, size).empty());
    }
    REQUIRE_FALSE(memory.IsFullyMappedRange(end - 1, 1));
    REQUIRE_FALSE(memory.GpuToCpuAddress(end));

    for (const GPUVAddr address : {end, end - 1, std::numeric_limits<u64>::max()}) {
        std::array<u8, 8> bytes;
        bytes.fill(0xff);
        memory.ReadBlock(address, bytes.data(), bytes.size());
        REQUIRE(std::ranges::all_of(bytes, [](u8 value) { return value == 0; }));
        bytes.fill(0xff);
        memory.ReadBlockUnsafe(address, bytes.data(), bytes.size());
        REQUIRE(std::ranges::all_of(bytes, [](u8 value) { return value == 0; }));
    }
}

TEST_CASE("GPU address translations follow page remaps and unmaps", "[gpu_command_safety]") {
    ScopedSystemTestContext context;
    Core::System system;
    Core::DeviceMemory physical;
    Tegra::MaxwellDeviceMemoryManager device_memory{physical};
    Tegra::MemoryManager memory{system, device_memory, 32, 1ULL << 31};
    Null::RasterizerNull rasterizer{system.GPU()};
    memory.BindRasterizer(&rasterizer);

    constexpr GPUVAddr gpu_page = 0x10000;
    constexpr GPUVAddr gpu_offset = 0x120;
    constexpr DAddr first_device_page = 0x2000;
    constexpr DAddr second_device_page = 0x9000;
    constexpr size_t page_size = 1ULL << 12;
    constexpr GPUVAddr big_gpu_page = 0x20000;
    constexpr DAddr big_device_page = 0x100000;
    constexpr size_t big_page_size = 1ULL << 16;

    memory.Map(gpu_page, first_device_page, page_size, Tegra::PTEKind::INVALID, false);
    REQUIRE(memory.GpuToCpuAddress(gpu_page + gpu_offset) == first_device_page + gpu_offset);

    Tegra::MemoryManager other_memory{system, device_memory, 32, 1ULL << 31};
    other_memory.BindRasterizer(&rasterizer);
    other_memory.Map(gpu_page, second_device_page, page_size, Tegra::PTEKind::INVALID, false);
    REQUIRE(other_memory.GpuToCpuAddress(gpu_page + gpu_offset) == second_device_page + gpu_offset);
    REQUIRE(memory.GpuToCpuAddress(gpu_page + gpu_offset) == first_device_page + gpu_offset);

    memory.Map(gpu_page, second_device_page, page_size, Tegra::PTEKind::INVALID, false);
    REQUIRE(memory.GpuToCpuAddress(gpu_page + gpu_offset) == second_device_page + gpu_offset);

    memory.Unmap(gpu_page, page_size);
    REQUIRE_FALSE(memory.GpuToCpuAddress(gpu_page + gpu_offset));
    REQUIRE(other_memory.GpuToCpuAddress(gpu_page + gpu_offset) == second_device_page + gpu_offset);

    memory.Map(big_gpu_page, big_device_page, big_page_size, Tegra::PTEKind::INVALID, true);
    REQUIRE(memory.GpuToCpuAddress(big_gpu_page + gpu_offset) == big_device_page + gpu_offset);
}

TEST_CASE("Invalid GPU syncpoint increments leave valid counters and actions intact",
          "[gpu_command_safety]") {
    Tegra::Host1x::SyncpointManager manager;
    constexpr u32 last = Tegra::Host1x::SyncpointManager::NUM_MAX_SYNCPOINTS - 1;
    u32 guest_actions{};
    u32 host_actions{};
    manager.RegisterGuestAction(last, 1, [&] { ++guest_actions; });
    manager.RegisterHostAction(last, 1, [&] { ++host_actions; });

    for (const u32 invalid : {last + 1, 0x80000e1cU, std::numeric_limits<u32>::max()}) {
        manager.IncrementGuest(invalid);
        manager.IncrementHost(invalid);
    }
    for (u32 id = 0; id <= last; ++id) {
        REQUIRE(manager.GetGuestSyncpointValue(id) == 0);
        REQUIRE(manager.GetHostSyncpointValue(id) == 0);
    }
    REQUIRE(guest_actions == 0);
    REQUIRE(host_actions == 0);

    manager.IncrementGuest(last);
    manager.IncrementHost(last);
    REQUIRE(manager.GetGuestSyncpointValue(last) == 1);
    REQUIRE(manager.GetHostSyncpointValue(last) == 1);
    REQUIRE(guest_actions == 1);
    REQUIRE(host_actions == 1);
}
