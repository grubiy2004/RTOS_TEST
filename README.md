# STM32 Project RTOS_TEST

## Contents

- [Contents](#contents)
- [Usage](#usage)
  - [Prerequisites](#prerequisites)
  - [Build system commands](#build-system-commands)
- [Features](#features)
  - [Build system](#build-system)
  - [Documentation](#documentation)
  - [Format check](#format-check)
  - [Static code analysis \& linting](#static-code-analysis--linting)
  - [Style check](#style-check)
  - [Continuous integration](#continuous-integration)
  - [Development container](#development-container)
  - [VSCode integration](#vscode-integration)
- [Repository structure](#repository-structure)

## Usage

### Prerequisites

- [CMake](https://cmake.org/download/) is installed and available on your PATH.
- [Ninja](https://ninja-build.org) is installed and available on your PATH.
  Alternatively, you can use Make.
- [GCC for ARM](https://developer.arm.com/downloads/-/gnu-rm) (GNU Arm Embedded
  Toolchain) is installed and available in your PATH.
- [Doxygen](https://www.doxygen.nl/download.html) is installed and available in your PATH.
- [OpenOCD](https://gnutoolchains.com/arm-eabi/openocd/) is installed and available in your PATH.

### Prerequisites in VS Code

Install extensions which VS Code will recommend (by extensions.json).

### Build system commands

Command                                               | Description
-|-
 `cmake --list-presets`                               | List all CMake presets
 `cmake --preset Debug`                               | Configure the project for Debug build
 `cmake --build --preset Debug`                       | Build the firmware with Debug build type
 `cmake --build --preset Debug --target clean`        | Clean the Debug target
 `cmake --build --preset Debug --target doxygen`      | Generate documentation with Doxygen

Supported CMake configurations and build presets:

| Preset           | Description                                                                         |
|:-----------------|:------------------------------------------------------------------------------------|
| `Debug`          | Debug preset for debugging, without any optimization enabled                        |
| `Release`        | Release preset with `O3` optimization                                               |
| `MinSizeRel`     | Release preset with `Os` optimization for size with link time optimization enabled  |
| `RelWithDebInfo` | Release preset with `O2` optimization with debug information                        |

## Features

### Documentation

The source code is documented with Javadoc style comment blocks. The
documentation output is generated with Doxygen.

## Repository structure

```text
├── .vscode
├── build
├── cmake
│   ├── microcontrollers
│   ├── toolchains
│   └── tools
├── docs
│   └── doxygen
├── include
├── lib
│   ├── CMSIS
│   └── cmsis-config
|   |-- EventRecorder
|   |-- FreeRTOS
├── mcal
│   └── st-stm32f103
│       ├── gcc-arm
│       ├── include
│       ├── source
│       └── svd
└── source
```

The `.github` folder contains the GitHub Actions workflow file which describes
the CI pipeline that runs automatically on every git push operation.

Upon building the project, a `build` folder is created. All build-related files
and output binaries are located in the `build` folder, organized into
subfolders. Each build target and their respective output files have their own
subfolder. The generated Doxygen documentation output files are also located in
the `build` folder.

The `cmake` folder contains the files related to the CMake-based build system,
including the toolchain and microcontroller-specific files.

The `docs` folder contains the doxygen configuration file (Doxyfile) and other
documentation-related static files.

The application-level source code and corresponding header files are located in
the `source` and `include` folders respectively.

The `lib` folder contains all third-party code, including the CMSIS (Cortex
Microcontroller Software Interface Standard) as well as the HAL (Hardware
Abstraction Layer) drivers from ST.

The `mcal` folder stands for Microcontroller Abstraction Library. This folder
contains the microcontroller-specific files and drivers. These drivers are
interfaced by the application source code and they function as tiny wrappers
around the low-level (HAL) drivers. This allows the application to interface
these thin wrappers instead of the manufacturer-specific low-level code, thus
providing easy portability across different chips and microcontrollers.
