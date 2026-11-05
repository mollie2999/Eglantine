// SPDX-License-Identifier: GPL-3.0-or-later

#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>

#include <catch2/catch_test_macros.hpp>

#include "shader_recompiler/program_header.h"
#include "video_core/shader_environment.h"

namespace {

std::filesystem::path UniqueTestPath() {
    const auto nonce = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           ("eglantine-mesh-environment-" + std::to_string(nonce) + ".bin");
}

void WriteMeshEnvironment(std::ofstream& file, const std::array<u32, 3>& registers,
                          const Shader::ProgramHeader& primitive_header) {
    const u64 code_size{sizeof(u64)};
    const u64 no_entries{};
    const u32 no_memory{};
    const u32 texture_bound{~0U};
    const u32 start_address{};
    const u32 read_lowest{};
    const u32 read_highest{};
    const u32 viewport_transform_state{1};
    const Shader::Stage stage{Shader::Stage::Mesh};
    const u64 code{0xe30000000007000FULL};
    const Shader::ProgramHeader sph{};

    file.write(reinterpret_cast<const char*>(&code_size), sizeof(code_size));
    for (size_t index{}; index < 5; ++index) {
        file.write(reinterpret_cast<const char*>(&no_entries), sizeof(no_entries));
    }
    file.write(reinterpret_cast<const char*>(&no_memory), sizeof(no_memory))
        .write(reinterpret_cast<const char*>(&texture_bound), sizeof(texture_bound))
        .write(reinterpret_cast<const char*>(&start_address), sizeof(start_address))
        .write(reinterpret_cast<const char*>(&read_lowest), sizeof(read_lowest))
        .write(reinterpret_cast<const char*>(&read_highest), sizeof(read_highest))
        .write(reinterpret_cast<const char*>(&viewport_transform_state),
               sizeof(viewport_transform_state))
        .write(reinterpret_cast<const char*>(&stage), sizeof(stage))
        .write(reinterpret_cast<const char*>(&code), sizeof(code))
        .write(reinterpret_cast<const char*>(&sph), sizeof(sph))
        .write(reinterpret_cast<const char*>(registers.data()), sizeof(registers))
        .write(reinterpret_cast<const char*>(&primitive_header), sizeof(primitive_header));
}

} // Anonymous namespace

TEST_CASE("Mesh environments retain their translation state in the disk cache",
          "[shader_environment]") {
    const std::array registers{0x0012'3454U, 0x03c0'0000U, 0x0100'1004U};
    Shader::ProgramHeader primitive_header{};
    primitive_header.common4.max_output_vertices.Assign(123);
    primitive_header.common4.store_req_start.Assign(7);
    primitive_header.common4.store_req_end.Assign(11);

    const auto path = UniqueTestPath();
    {
        std::ofstream file{path, std::ios::binary};
        REQUIRE(file.is_open());
        WriteMeshEnvironment(file, registers, primitive_header);
    }

    VideoCommon::FileEnvironment env;
    {
        std::ifstream file{path, std::ios::binary};
        REQUIRE(file.is_open());
        env.Deserialize(file);
    }
    std::filesystem::remove(path);

    REQUIRE(env.ShaderStage() == Shader::Stage::Mesh);
    REQUIRE(env.MeshRegisters() == registers);
    REQUIRE(env.MeshPrimitiveHeader().common4.max_output_vertices ==
            primitive_header.common4.max_output_vertices);
    REQUIRE(env.MeshPrimitiveHeader().common4.store_req_start ==
            primitive_header.common4.store_req_start);
    REQUIRE(env.MeshPrimitiveHeader().common4.store_req_end ==
            primitive_header.common4.store_req_end);
}
