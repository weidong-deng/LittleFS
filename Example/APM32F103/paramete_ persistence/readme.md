# LittleFS Parameter Persistence Demo

***

## English

### Overview

This demo demonstrates how to use LittleFS file system to persist device configuration parameters in the APM32F103 internal flash. It shows how structured parameter data can be saved, loaded, and validated across power cycles using CRC32 integrity checks.

### Key Features

- **Parameter Structure**: A `DeviceParams_t` structure containing device configuration fields:
  - Magic number (`0xDEADBEEF`) for validity detection
  - Boot counter (increments each power cycle)
  - Device ID
  - Calibration offset
  - Operating voltage (mV)
  - Device name string (32 bytes)
  - CRC32 checksum for data integrity
- **Data Integrity**: CRC32 checksum is calculated before every write and verified on every read to detect corrupted data.
- **Auto Initialization**: On first run (or after flash erase), default parameters are automatically created and saved.
- **Read-back Verification**: After each write, data is read back and compared to ensure correctness.
- **Multi-boot Simulation**: The demo simulates 10 consecutive boot cycles, persisting parameters after each one.

### How It Works

1. Mount LittleFS on internal flash (0x08010000, 64KB).
2. Attempt to load `/device_params.bin` from flash.
3. If no valid file is found (first run), initialize default parameters and save.
4. Increment boot counter and modify parameters to simulate a real boot cycle.
5. Save updated parameters to flash.
6. Read back and verify data integrity via CRC32.
7. Repeat for 10 simulated boot cycles.
8. Perform final verification and print summary.

### USART1 Output

USART1 is configured at **115200 baud, 8N1**. The following output is expected on first run:

```
=== LittleFS Parameter Persistence Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64

[Test 1] Load parameters from flash
  No valid parameters found (err=-2), initializing defaults
  Default parameters:
  Magic:        0xDEADBEEF
  Boot Count:   0
  Device ID:    0x00A0F103
  Calibration:  -50
  Voltage:      3300 mV
  Device Name:  APM32F103VB
  CRC32:        0xA5DF33E0
  Default parameters saved to flash

[Test 2] Simulate boot cycle (increment & persist)
  Updated parameters saved (boot #1)

[Test 3] Read back and verify data integrity
  PASS: Read-back data matches written data

[Test 4] Simulate 10 boot cycles
  10 boot cycles completed successfully
  Final boot count: 11

[Test 5] Final verification
  Final parameters:
  Magic:        0xDEADBEEF
  Boot Count:   11
  Device ID:    0x00A0F103
  Calibration:  -39
  Voltage:      3310 mV
  Device Name:  APM32F103VB
  CRC32:        0x5A1988C9
  PASS: Parameters persisted correctly

=== Parameter Persistence Summary ===
Errors: 0
Result: PASS

Note: Reset the board to see boot_count increment,
      demonstrating parameter persistence across power cycles.

=== Demo Complete ===
```

### Verifying Persistence

To confirm parameters survive a power cycle:

1. Run the demo once — note the final `Boot Count` value.
2. Reset or power-cycle the board.
3. Run the demo again — `[Test 1]` should show `Parameters loaded successfully` with the `Boot Count` resuming from the previous value, proving data was persisted in flash.

### Hardware Requirements

- APM32F103 MINI board
- USART1 TX pin (PA9) connected to USB-to-UART adapter
- Internal flash area: 0x08010000 - 0x0801FFFF (64KB, block size 1KB)

### IDE Support

- MDK-ARM V5.36
- EWARM V8.50.5.26295
- Eclipse V2022-06 (4.24.0)

***

## 中文

### 概述

本 Demo 演示如何使用 LittleFS 文件系统将设备配置参数持久化存储到 APM32F103 的内部 Flash 中。通过 CRC32 校验机制，展示结构化参数数据如何在掉电重启后依然保持完整和有效。

### 主要特性

- **参数结构体**：`DeviceParams_t` 结构体包含以下设备配置字段：
  - 魔数（`0xDEADBEEF`）用于有效性检测
  - 启动计数器（每次上电递增）
  - 设备 ID
  - 校准偏移值
  - 工作电压（mV）
  - 设备名称字符串（32 字节）
  - CRC32 校验和用于数据完整性验证
- **数据完整性**：每次写入前自动计算 CRC32 校验和，每次读取时进行验证，防止数据损坏。
- **自动初始化**：首次运行（或 Flash 擦除后），自动创建并保存默认参数。
- **回读验证**：每次写入后回读数据并比对，确保写入正确。
- **多次启动模拟**：Demo 模拟 10 次连续启动循环，每次循环后持久化参数。

### 工作流程

1. 在内部 Flash（0x08010000，64KB）上挂载 LittleFS。
2. 尝试从 Flash 加载 `/device_params.bin`。
3. 如果未找到有效文件（首次运行），初始化默认参数并保存。
4. 递增启动计数器并修改参数，模拟真实启动过程。
5. 将更新后的参数保存到 Flash。
6. 回读并通过 CRC32 验证数据完整性。
7. 重复 10 次模拟启动循环。
8. 执行最终验证并打印摘要。

### USART1 输出

USART1 配置为 **115200 波特率，8N1**。首次运行的预期输出如下：

```
=== LittleFS Parameter Persistence Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64

[Test 1] Load parameters from flash
  No valid parameters found (err=-2), initializing defaults
  Default parameters:
  Magic:        0xDEADBEEF
  Boot Count:   0
  Device ID:    0x00A0F103
  Calibration:  -50
  Voltage:      3300 mV
  Device Name:  APM32F103VB
  CRC32:        0xA5DF33E0
  Default parameters saved to flash

[Test 2] Simulate boot cycle (increment & persist)
  Updated parameters saved (boot #1)

[Test 3] Read back and verify data integrity
  PASS: Read-back data matches written data

[Test 4] Simulate 10 boot cycles
  10 boot cycles completed successfully
  Final boot count: 11

[Test 5] Final verification
  Final parameters:
  Magic:        0xDEADBEEF
  Boot Count:   11
  Device ID:    0x00A0F103
  Calibration:  -39
  Voltage:      3310 mV
  Device Name:  APM32F103VB
  CRC32:        0x5A1988C9
  PASS: Parameters persisted correctly

=== Parameter Persistence Summary ===
Errors: 0
Result: PASS

Note: Reset the board to see boot_count increment,
      demonstrating parameter persistence across power cycles.

=== Demo Complete ===
```

### 验证持久化效果

确认参数能经受掉电重启的方法：

1. 运行一次 Demo，记录最终的 `Boot Count` 值。
2. 复位或断电重启开发板。
3. 再次运行 Demo — `[Test 1]` 应显示 `Parameters loaded successfully`，且 `Boot Count` 从上次的值继续递增，证明数据已持久化到 Flash 中。

### 硬件需求

- APM32F103 MINI 开发板
- USART1 TX 引脚（PA9）连接 USB 转串口模块
- 内部 Flash 区域：0x08010000 - 0x0801FFFF（64KB，块大小 1KB）

### IDE 支持

- MDK-ARM V5.36
- EWARM V8.50.5.26295
- Eclipse V2022-06（4.24.0）

