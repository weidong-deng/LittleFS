# LittleFS 测试框架

基于 Unity 测试框架的 LittleFS 文件系统 Host 测试套件，在 PC 上运行，无需目标硬件。

## 目录结构

```
test/
├── Unity/                              # Unity 测试框架
│   └── src/
│       ├── unity.c
│       ├── unity.h
│       └── unity_internals.h
├── stubs/                              # Stub 实现（替代硬件依赖）
│   ├── lfs_flash_int_port_stub.c       # 内部 Flash 端口 stub（用 rambd 模拟）
│   └── lfs_spi_flash_port_stub.c       # 外部 SPI Flash 端口 stub
├── test_lfs_core/                      # LittleFS 核心库测试
│   ├── test_lfs_format.c               # 格式化测试（4 个用例）
│   ├── test_lfs_file.c                 # 文件操作测试（22 个用例）
│   ├── test_lfs_dir.c                  # 目录操作测试（9 个用例）
│   └── test_lfs_fs.c                   # 文件系统信息测试（15 个用例）
├── test_lfs_app/                       # 应用层测试
│   ├── test_lfs_app_init.c             # 初始化与生命周期测试（10 个用例）
│   ├── test_lfs_app_file.c             # 文件操作测试（16 个用例）
│   ├── test_lfs_app_dir.c              # 目录操作测试（8 个用例）
│   ├── test_lfs_app_info.c             # 信息查询测试（7 个用例）
│   └── test_lfs_app_dual_storage.c     # 双存储同时操作测试（9 个用例）
├── test_block_device/                  # 块设备测试
│   ├── test_rambd.c                    # RAM 块设备测试（9 个用例）
│   └── test_emubd.c                    # 模拟块设备测试（7 个用例）
├── test_port/                          # 端口层测试
│   ├── test_spi_flash_port.c           # SPI Flash 端口测试（6 个用例）
│   └── test_int_flash_port.c           # 内部 Flash 端口测试（5 个用例）
├── test_fault_injection/               # 异常注入测试（使用 lfs_emubd）
│   ├── test_device_fault.c             # 设备故障测试（5 个用例）
│   ├── test_fs_corruption.c            # 文件系统损坏测试（5 个用例）
│   ├── test_power_loss.c               # 电源故障模拟测试（5 个用例）
│   └── test_wear_leveling.c            # 磨损均衡验证测试（4 个用例）
├── common/                             # 测试公共代码
│   ├── test_helpers.c
│   └── test_helpers.h
├── CMakeLists.txt                      # CMake 构建脚本
├── Makefile                            # Make 构建脚本
├── build.bat                           # Windows 构建脚本
├── build.sh                            # Linux 构建脚本
└── TEST_FRAMEWORK_PLAN.md              # 测试框架设计文档
```

## 快速开始

### Windows

```bash
cd test
build.bat
```

### Linux

```bash
cd test
chmod +x build.sh
./build.sh
```

### 使用 CMake

```bash
cd test
mkdir build && cd build
cmake ..
make -j8
ctest --output-on-failure
```

### 使用 Make

```bash
cd test
make all
make test
```

## 测试模块说明

| 模块 | 可执行文件数 | 用例数 | 说明 |
|------|-------------|--------|------|
| 核心库 | 4 | 50 | LittleFS 核心 API 测试 |
| 应用层 | 5 | 50 | LFS_App_* 系列函数测试 |
| 块设备 | 2 | 16 | rambd/emubd 块设备测试 |
| 端口层 | 2 | 11 | Flash 端口层测试 |
| 异常注入 | 4 | 19 | 故障/损坏/掉电/磨损测试 |
| **总计** | **17** | **146** | |

## 构建目标

### CMake 目标

```bash
cmake --build . --target run_all_tests      # 运行所有测试
cmake --build . --target coverage           # 生成覆盖率报告
```

### Make 目标

```bash
make all            # 编译所有测试
make test           # 运行所有测试
make coverage       # 生成覆盖率报告
make clean          # 清理构建文件
```

## 测试架构

```
┌─────────────────────────────────────────┐
│           测试文件 (test_*.c)            │  ← 每个文件自带 main()，独立编译
├─────────────────────────────────────────┤
│           公共测试代码 (test_helpers)     │  ← 测试辅助函数
├─────────────────────────────────────────┤
│           Stub 实现 (stubs/)             │  ← 替代硬件端口层
├─────────────────────────────────────────┤
│           LittleFS 核心库 (lfs.c)        │  ← 被测代码
├─────────────────────────────────────────┤
│           块设备层 (rambd/emubd)         │  ← 被测代码 + 模拟块设备
├─────────────────────────────────────────┤
│           应用层 (lfs_app.c)             │  ← 被测代码
─────────────────────────────────────────┤
│           Unity 测试框架                 │  ← 断言和测试管理
└─────────────────────────────────────────┘
```

## 设计决策

- **每个测试文件独立编译为可执行文件**：每个文件自带 `main()`，避免多个 `setUp`/`tearDown` 符号冲突
- **使用 stub 替代硬件**：内部 Flash 用 rambd 模拟，外部 SPI Flash 通过注入机制模拟
- **异常注入使用 lfs_emubd**：直接使用 LittleFS 自带的模拟块设备，不自研故障注入代码
- **LFS_ASSERT 失败时退出**：定义 `LFS_ASSERT_TEST_FAIL=1`，避免断言失败时死循环
- **覆盖率编译选项**：使用 `--coverage` 编译，支持 gcovr 生成覆盖率报告

## 覆盖率报告

运行测试后生成覆盖率报告：

```bash
# CMake 方式
cmake --build . --target coverage

# Make 方式
make coverage

# build.bat / build.sh 会自动生成
```

报告输出到 `build/coverage_report/coverage.html`（CMake）或 `build/coverage/coverage_report.html`（build.bat/build.sh）。
