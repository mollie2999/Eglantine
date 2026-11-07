// SPDX-License-Identifier: GPL-3.0-or-later

#include <bit>
#include <tuple>
#include <utility>
#include <catch2/catch_test_macros.hpp>

#include "video_core/buffer_cache/buffer_cache_base.h"

TEST_CASE("Buffer cache holds every shader storage binding", "[video_core][storage_bindings]") {
    using Channel = VideoCommon::BufferCacheChannelInfo;
    using ComputeBindings = decltype(std::declval<Channel>().compute_storage_buffers);
    using GraphicsBindings = decltype(std::declval<Channel>().storage_buffers)::value_type;
    using ComputeMask = decltype(std::declval<Channel>().enabled_compute_storage_buffers);
    using GraphicsMask = decltype(std::declval<Channel>().enabled_storage_buffers)::value_type;
    STATIC_REQUIRE(std::tuple_size_v<ComputeBindings> == Shader::Info::MAX_SSBOS);
    STATIC_REQUIRE(std::tuple_size_v<GraphicsBindings> == Shader::Info::MAX_GRAPHICS_SSBOS);
    STATIC_REQUIRE(Shader::Info::MAX_SSBOS <= sizeof(ComputeMask) * 8);
    STATIC_REQUIRE(Shader::Info::MAX_GRAPHICS_SSBOS <= sizeof(GraphicsMask) * 8);

    ComputeBindings compute{};
    GraphicsBindings graphics{};
    ComputeMask enabled{};
    for (u32 index = 0; index < Shader::Info::MAX_SSBOS; ++index) {
        compute.at(index).size = index + 1;
        if (index < Shader::Info::MAX_GRAPHICS_SSBOS) {
            graphics.at(index).size = index + 1;
        }
        enabled |= ComputeMask{1} << index;
    }
    // Bananza's 18-buffer compute shader previously lost slots 16 and 17,
    // shifting the image descriptors that followed them.
    REQUIRE(compute.at(16).size == 17);
    REQUIRE(compute.at(17).size == 18);
    // Ocarina of Time's foliage list builder binds 39.
    REQUIRE(compute.at(38).size == 39);
    u32 count{};
    while (enabled != 0) {
        const auto index = static_cast<u32>(std::countr_zero(enabled));
        REQUIRE(compute.at(index).size == index + 1);
        if (index < Shader::Info::MAX_GRAPHICS_SSBOS) {
            REQUIRE(graphics.at(index).size == index + 1);
        }
        enabled &= enabled - 1;
        ++count;
    }
    REQUIRE(count == Shader::Info::MAX_SSBOS);
}
