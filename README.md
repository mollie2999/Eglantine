<!--
# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: 2018 yuzu Emulator Project
# SPDX-License-Identifier: GPL-2.0-or-later
-->

<p align="center">
    </a>
    <a href="https://discord.gg/XkS5NVPTUY">
        <img src="https://img.shields.io/discord/1367654015269339267?color=5865F2&label=Eglantine&logo=discord&logoColor=white"
            alt="Discord">
    </a>
  
</p>

# Eglantine

Eglantine is an experimental, open-source Nintendo Switch and Switch 2 emulator written in C++. It builds on upstream Eglantine, Eden, yuzu and Sudachi, preserving their contributions while developing this fork's game compatibility and renderer fixes.

This is an independent fork, not an official release of those upstream projects. Eglantine is not affiliated with Nintendo.

## Downloads and setup

Published builds are on this fork's [releases page](https://github.com/mollie2999/Eglantine/releases).

On Windows, extract the entire build archive and run **eglantine.exe**. Only use games and system files you have legally obtained. Games, keys and firmware are not included.

### Existing saves and settings

Builds keep using the existing Eden/Eglantine data directories and configuration keys. They do not move or delete saves, settings or shader caches. Back up your saves before testing experimental builds.

## Compatibility and issue reports

Compatibility and performance vary by game, hardware and driver. Rendering errors, stutters and crashes remain possible. Passing automated tests does not establish that a game renders correctly, and this project makes no universal frame-rate guarantee.

When [reporting an issue](https://github.com/mollie2999/Eglantine/issues), include:

- The exact build, game version, CPU, GPU and driver version.
- Relevant settings and steps to reproduce the problem.
- A screenshot and the log from the affected session, after checking it for personal information.

Do not upload game files, keys, firmware or private account information.

## Building and contributing

See the [build guide](docs/Build.md), [user documentation](docs/user/README.md) and [coding guidelines](docs/Coding.md). Some inherited documentation and internal build target names still use upstream names. The GUI build target remains `yuzu`; its output is `eglantine.exe` on Windows and `eglantine` on other desktop platforms.

Keep changes focused and distinguish code-level checks from actual gameplay testing.

## Artwork
Art created by YOB (yobslab)
The supplied logo, icon and library artwork are in [dist/branding](dist/branding). Windows, Linux and macOS icon resources are generated from that artwork by the script in [tools](tools).

## Credits and license

Eglantine inherits the work of the Eglantine, Eden, yuzu and Sudachi projects and their contributors, including Eglantine's original developer Camille LaVey. Original copyright notices and third-party licenses are retained.

See [LICENSE.txt](LICENSE.txt) and the source files for applicable license terms.
