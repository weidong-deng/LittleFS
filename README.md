# LittleFS MCU Port

[English](#english) | [中文](#中文)

***

<a id="english"></a>

# English

## Overview

This project ports [LittleFS](https://github.com/littlefs-project/littlefs) — a small, fail-safe filesystem designed for microcontrollers — to MCU platforms. It supports both internal Flash and external SPI Flash as storage backends.

## Supported MCUs

| MCU | Status | Notes |
| --- | :----: | ----- |
| APM32F103 | ✅ | Internal Flash + SPI Flash |
| APM32F107 | ✅ | Internal Flash + SPI Flash |

## Project Structure

```
Code/
├── Littlefs/                    # LittleFS source code + port layer
│   ├── lfs.c / lfs.h            # Core filesystem
│   ├── lfs_util.c / lfs_util.h  # Utility functions
│   ├── bd/                      # Block device drivers (rambd, emubd)
│   └── port/                    # MCU port layer (lfs_app.c, flash drivers)
├── Boards/                      # Board support
├── Libraries/                   # CMSIS, DSP, RTOS libraries
├── Example/                     # Example projects
└── test/                        # Host-side test suite
    ├── Unity/                   # Unity test framework
    ├── stubs/                   # Hardware stubs for host testing
    ├── common/                  # Shared test helpers
    ├── test_lfs_core/           # Core library tests
    ├── test_lfs_app/            # Application layer tests
    ├── test_block_device/       # Block device tests
    ├── test_port/               # Port layer tests
    ── test_fault_injection/    # Fault injection tests
```

## Prerequisites

### Target Firmware Build

| Tool | Version | Download |
| --- | --- | --- |
| ARM GCC Toolchain | ≥ 10.0 | [ARM Developer](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) |
| CMake | ≥ 3.10 | [cmake.org](https://cmake.org/download/) |
| Make / Ninja | any | [GNU Make](https://www.gnu.org/software/make/) / [Ninja](https://ninja-build.org/) |
| Eclipse *(optional)* | latest | [Eclipse IDE for Embedded](https://www.eclipse.org/downloads/packages/release/latest/r/eclipse-ide-embedded-developers) |

### Host Tests (PC)

| Tool | Version | Download |
| --- | --- | --- |
| GCC (MinGW-w64 on Windows) | ≥ 10.0 | [MinGW-w64](https://www.mingw-w64.org/downloads/) |
| CMake | ≥ 3.10 | [cmake.org](https://cmake.org/download/) |
| gcovr | ≥ 5.0 | [gcovr.com](https://gcovr.com/en/stable/installation.html) |

### Quick Install

**Windows:**
```bash
# Install MinGW-w64 (via MSYS2 or standalone)
# Install CMake from https://cmake.org/download/
# Install Python + gcovr
pip install gcovr
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install gcc cmake gcovr
```

**macOS:**
```bash
brew install gcc cmake gcovr
```

## Build & Test

### Host Tests

```bash
cd test

# Windows
build.bat

# Linux
chmod +x build.sh && ./build.sh

# Or using CMake directly
mkdir build && cd build
cmake ..
make -j8
ctest --output-on-failure

# Generate coverage report
make coverage
```

### Target Firmware

Build with your preferred toolchain (Keil, IAR, or GCC) targeting a supported MCU. See the `Example/` directory for ready-to-use project files.

## Flash Configuration

| Parameter | Value |
| --- | --- |
| Internal Flash base | `0x08010000` |
| Flash size | 64 KB |
| Block size | 1 KB |
| Block count | 64 |

## License

- LittleFS: [BSD-3-Clause](https://github.com/littlefs-project/littlefs/blob/master/LICENSE.md)
- Unity Test Framework: [MIT](https://github.com/ThrowTheSwitch/Unity/blob/master/LICENSE.txt)

***

<a id="中文"></a>

# 中文

## 项目简介

本项目将 [LittleFS](https://github.com/littlefs-project/littlefs)（一个为微控制器设计的小型、掉电安全文件系统）移植到 MCU 平台。支持内部 Flash 和外部 SPI Flash 作为存储后端。

## 支持的 MCU

| MCU | 状态 | 说明 |
| --- | :---: | --- |
| APM32F103 | ✅ | 内部 Flash + SPI Flash |
| APM32F107 | ✅ | 内部 Flash + SPI Flash |

## 项目结构

```
Code/
├── Littlefs/                    # LittleFS 源码 + 移植层
│   ├── lfs.c / lfs.h            # 核心文件系统
│   ├── lfs_util.c / lfs_util.h  # 工具函数
│   ├── bd/                      # 块设备驱动（rambd、emubd）
│   └── port/                    # MCU 移植层（lfs_app.c、Flash 驱动）
├── Boards/                      # 板级支持
├── Libraries/                   # CMSIS、DSP、RTOS 库
├── Example/                     # 示例工程
└── test/                        # PC 端测试套件
    ├── Unity/                   # Unity 测试框架
    ├── stubs/                   # 硬件桩（替代真实硬件）
    ├── common/                  # 公共测试辅助代码
    ├── test_lfs_core/           # 核心库测试
    ├── test_lfs_app/            # 应用层测试
    ├── test_block_device/       # 块设备测试
    ├── test_port/               # 端口层测试
    └── test_fault_injection/    # 故障注入测试
```

## 环境依赖

### 编译目标固件

| 工具 | 版本要求 | 下载地址 |
| --- | --- | --- |
| ARM GCC 工具链 | ≥ 10.0 | [ARM Developer](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) |
| CMake | ≥ 3.10 | [cmake.org](https://cmake.org/download/) |
| Make / Ninja | 任意 | [GNU Make](https://www.gnu.org/software/make/) / [Ninja](https://ninja-build.org/) |
| Eclipse *（可选）* | 最新版 | [Eclipse IDE for Embedded](https://www.eclipse.org/downloads/packages/release/latest/r/eclipse-ide-embedded-developers) |

### 运行 PC 端测试

| 工具 | 版本要求 | 下载地址 |
| --- | --- | --- |
| GCC（Windows 用 MinGW-w64） | ≥ 10.0 | [MinGW-w64](https://www.mingw-w64.org/downloads/) |
| CMake | ≥ 3.10 | [cmake.org](https://cmake.org/download/) |
| gcovr（生成覆盖率报告） | ≥ 5.0 | [gcovr.com](https://gcovr.com/en/stable/installation.html) |

### 快速安装

**Windows：**
```bash
# 安装 MinGW-w64（可通过 MSYS2 或独立安装）
# 从 https://cmake.org/download/ 安装 CMake
# 安装 Python + gcovr
pip install gcovr
```

**Linux（Ubuntu/Debian）：**
```bash
sudo apt update
sudo apt install gcc cmake gcovr
```

**macOS：**
```bash
brew install gcc cmake gcovr
```

## 编译与测试

### PC 端测试

```bash
cd test

# Windows
build.bat

# Linux
chmod +x build.sh && ./build.sh

# 或直接用 CMake
mkdir build && cd build
cmake ..
make -j8
ctest --output-on-failure

# 生成覆盖率报告
make coverage
```

### 目标固件

使用 Keil、IAR 或 GCC 工具链编译，目标芯片为已支持的 MCU。参见 `Example/` 目录下的现成工程文件。

## Flash 配置

| 参数 | 值 |
| --- | --- |
| 内部 Flash 起始地址 | `0x08010000` |
| Flash 大小 | 64 KB |
| 块大小 | 1 KB |
| 块数量 | 64 |

## 许可证

- LittleFS：[BSD-3-Clause](https://github.com/littlefs-project/littlefs/blob/master/LICENSE.md)
- Unity 测试框架：[MIT](https://github.com/ThrowTheSwitch/Unity/blob/master/LICENSE.txt)
