# LittleFS Power Loss Resilience Demo

## English

### Overview

This demo demonstrates the power loss resilience capability of LittleFS on APM32F103 internal flash. It uses KEY1 button to simulate sudden power loss during write operations, verifying that data remains consistent and the file system can recover gracefully.

### Key Features

- **Two-Phase Design**:
  - **Phase 1 - Recovery Verification**: Mounts the file system, reads counter and data files, verifies CRC32 and byte pattern integrity
  - **Phase 2 - Continuous Write**: Continuously writes incrementing counter + data blocks in a loop
- **Data Integrity Verification**: Each 512-byte data block contains sequence number, CRC32 checksum, and fill pattern
- **Power Loss Simulation**: KEY1 triggers immediate system reset (NVIC_SystemReset) without unmounting, simulating real power failure
- **Graceful Stop**: KEY2 stops the write loop and unmounts the file system normally

### Data Block Structure

```c
typedef struct {
    uint32_t seq;       /* Sequence number (incremented each write) */
    uint32_t crc32;     /* CRC32 of payload */
    uint8_t  payload[504]; /* Fill pattern: all bytes = (seq & 0xFF) */
} DataBlock_t;          /* Total: 512 bytes */
```

### Test Procedure

1. Power on / reset the board
2. **Phase 1** verifies any previously written data (first boot: no data)
3. **Phase 2** starts continuous writing
4. Press **KEY1** at any time to simulate power loss
5. Board resets, go back to step 2
6. Observe that Phase 1 always passes, proving data integrity

### USART1 Output

USART1 is configured at **115200 baud, 8N1**.

**First Boot (No Previous Data):**

```
=== LittleFS Power Loss Resilience Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64

Press KEY1 to simulate power loss during write operations
KEY2: Stop continuous write


=== Phase 1: Recovery Verification ===
  [PASS] Filesystem mounted successfully
  [INFO] No counter file found -- first boot

=== Phase 2: Continuous Write (press KEY1 to cut power) ===
  Starting fresh from counter: 0

  Writing continuously... Press KEY1 to simulate power loss.
  (KEY2 to stop gracefully)

  [10] seq=10 written OK
  [20] seq=20 written OK
  [30] seq=30 written OK
```

**After Pressing KEY1 (Simulated Power Loss):**

```
  >>> KEY1 PRESSED - CUTTING POWER NOW <<<
  Current seq: 27 (may or may not be persisted)
  !!! SIMULATED POWER LOSS !!!

=== LittleFS Power Loss Resilience Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64

Press KEY1 to simulate power loss during write operations
KEY2: Stop continuous write


=== Phase 1: Recovery Verification ===
  [PASS] Filesystem mounted successfully
  [PASS] Counter recovered: 27
  Data block: seq=27, crc32=0x3A7F2E19
  [PASS] Data integrity verified (CRC32 + pattern)
  [PASS] Counter (27) matches data seq (27)

  >>> Recovery verification PASSED <<<
  Filesystem survived power loss without corruption.

=== Phase 2: Continuous Write (press KEY1 to cut power) ===
  Resuming from counter: 27

  Writing continuously... Press KEY1 to simulate power loss.
  (KEY2 to stop gracefully)

  [10] seq=37 written OK
```

**After Pressing KEY2 (Graceful Stop):**

```
  >>> KEY2 PRESSED - Stopping gracefully <<<

=== Write Session Summary ===
  Total writes completed: 45
  Final counter value: 45

  >>> Reset the board now to verify data survived <<<
  >>> (or press KEY1 during next run to test again) <<<

=== Test Complete ===
```

### Hardware Requirements

- APM32F103 development board
- USART1 TX: PA9 (115200 baud, 8N1)
- KEY1: PA1 (active low, press to simulate power loss)
- KEY2: PA0 (active low, press to stop continuous write)
- Internal flash area: 0x08010000 - 0x0801FFFF (64KB)

---

## 中文

### 概述

本演示展示了 LittleFS 在 APM32F103 内部闪存上的掉电安全能力。它使用 KEY1 按键模拟写入操作期间的突然掉电，验证数据保持一致性并且文件系统能够优雅恢复。

### 主要特性

- **两阶段设计**：
  - **阶段一 - 恢复验证**：挂载文件系统，读取计数器和数据文件，验证 CRC32 和字节模式完整性
  - **阶段二 - 持续写入**：循环写入递增的计数器 + 数据块
- **数据完整性验证**：每个 512 字节数据块包含序列号、CRC32 校验和和填充模式
- **掉电模拟**：KEY1 触发立即系统复位（NVIC_SystemReset），不卸载文件系统，模拟真实掉电
- **优雅停止**：KEY2 停止写入循环并正常卸载文件系统

### 数据块结构

```c
typedef struct {
    uint32_t seq;       /* 序列号（每次写入递增） */
    uint32_t crc32;     /* payload 的 CRC32 校验和 */
    uint8_t  payload[504]; /* 填充模式：所有字节 = (seq & 0xFF) */
} DataBlock_t;          /* 总计：512 字节 */
```

### 测试流程

1. 上电/复位开发板
2. **阶段一**验证之前写入的数据（首次启动：无数据）
3. **阶段二**开始持续写入
4. 随时按下 **KEY1** 模拟掉电
5. 开发板复位，回到步骤 2
6. 观察阶段一始终通过，证明数据完整性

### USART1 输出

USART1 配置为 **115200 波特率，8N1**。

**首次启动（无历史数据）：**

```
=== LittleFS Power Loss Resilience Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64

Press KEY1 to simulate power loss during write operations
KEY2: Stop continuous write


=== Phase 1: Recovery Verification ===
  [PASS] Filesystem mounted successfully
  [INFO] No counter file found -- first boot

=== Phase 2: Continuous Write (press KEY1 to cut power) ===
  Starting fresh from counter: 0

  Writing continuously... Press KEY1 to simulate power loss.
  (KEY2 to stop gracefully)

  [10] seq=10 written OK
  [20] seq=20 written OK
  [30] seq=30 written OK
```

**按下 KEY1 后（模拟掉电）：**

```
  >>> KEY1 PRESSED - CUTTING POWER NOW <<<
  Current seq: 27 (may or may not be persisted)
  !!! SIMULATED POWER LOSS !!!

=== LittleFS Power Loss Resilience Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64

Press KEY1 to simulate power loss during write operations
KEY2: Stop continuous write


=== Phase 1: Recovery Verification ===
  [PASS] Filesystem mounted successfully
  [PASS] Counter recovered: 27
  Data block: seq=27, crc32=0x3A7F2E19
  [PASS] Data integrity verified (CRC32 + pattern)
  [PASS] Counter (27) matches data seq (27)

  >>> Recovery verification PASSED <<<
  Filesystem survived power loss without corruption.

=== Phase 2: Continuous Write (press KEY1 to cut power) ===
  Resuming from counter: 27

  Writing continuously... Press KEY1 to simulate power loss.
  (KEY2 to stop gracefully)

  [10] seq=37 written OK
```

**按下 KEY2 后（优雅停止）：**

```
  >>> KEY2 PRESSED - Stopping gracefully <<<

=== Write Session Summary ===
  Total writes completed: 45
  Final counter value: 45

  >>> Reset the board now to verify data survived <<<
  >>> (or press KEY1 during next run to test again) <<<

=== Test Complete ===
```

### 硬件要求

- APM32F103 开发板
- USART1 TX: PA9（115200 波特率，8N1）
- KEY1: PA1（低电平有效，按下模拟掉电）
- KEY2: PA0（低电平有效，按下停止持续写入）
- 内部闪存区域：0x08010000 - 0x0801FFFF（64KB）
