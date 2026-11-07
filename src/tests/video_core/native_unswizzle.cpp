// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <random>
#include <span>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "common/common_types.h"
#include "video_core/textures/decoders.h"

TEST_CASE("Native paired-row unswizzle matches the general path", "[video_core]") {
    std::mt19937 rng{0x1975};
    std::vector<u8> input(8 * 1024 * 1024);
    for (u8& byte : input) {
        byte = static_cast<u8>(rng());
    }

    for (int i = 0; i < 512; ++i) {
        const u32 width = 1 + rng() % 256;
        const u32 height = 1 + rng() % 96;
        const u32 bytes_per_pixel = 1U << (rng() % 5);
        const u32 depth = 1 + rng() % 3;
        const u32 block_height = rng() % 4;
        const u32 block_depth = rng() % 3;
        const u32 alignment = rng() % 4;
        const bool native = (rng() & 1) != 0;
        const size_t bytes = size_t{width} * height * depth * bytes_per_pixel;
        std::vector<u8> expected(bytes + 64, 0x5a);
        std::vector<u8> actual(bytes + 64, 0x5a);
        Tegra::Texture::UnswizzleTexture(expected, input, bytes_per_pixel, width, height, depth,
                                         block_height, block_depth, alignment, native);
        Tegra::Texture::UnswizzleTextureNativeFast(actual, input, bytes_per_pixel, width, height,
                                                   depth, block_height, block_depth, alignment,
                                                   native);
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Native paired-row unswizzle preserves overlapping buffer behavior", "[video_core]") {
    for (const size_t offset : {size_t{0}, size_t{16}, size_t{1024}, size_t{65536}}) {
        std::vector<u8> expected(2 * 1024 * 1024);
        for (size_t i = 0; i < expected.size(); ++i) {
            expected[i] = static_cast<u8>(i * 37 + (i >> 11));
        }
        std::vector<u8> actual = expected;
        const std::span<const u8> expected_input(expected.data(), 1024 * 1024);
        const std::span<const u8> actual_input(actual.data(), 1024 * 1024);
        const std::span<u8> expected_output(expected.data() + offset, 32768);
        const std::span<u8> actual_output(actual.data() + offset, 32768);
        Tegra::Texture::UnswizzleTexture(expected_output, expected_input, 4, 128, 64, 1, 2, 0, 0,
                                         true);
        Tegra::Texture::UnswizzleTextureNativeFast(actual_output, actual_input, 4, 128, 64, 1, 2, 0,
                                                   0, true);
        REQUIRE(actual == expected);
    }
}
