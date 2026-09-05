# LittleFS Circular Log Demo

## English

### Overview

This demo demonstrates how to implement a circular log buffer using LittleFS on APM32F103 internal flash. It shows how to create a log file, append log entries with timestamps, and automatically truncate the file when it exceeds a maximum size, simulating circular buffer behavior.

### Key Features

- **Log File Creation**: Initialize a log file on LittleFS
- **Timestamped Entries**: Append 30 log entries with sensor values and timestamps
- **Automatic Truncation**: File is automatically truncated when size exceeds LOG\_MAX\_SIZE (2048 bytes)
- **Log Reading**: Read and display complete log file contents
- **File System Statistics**: Report disk version, block usage, and available space
- **Practical Use Case**: Demonstrates embedded system logging with automatic size management

### How It Works

1. Mount LittleFS on internal flash (0x08010000, 64KB, block size 1KB)
2. Create a log file at `/circular.log`
3. Append 30 log entries, each containing:
   - Log entry number
   - Simulated sensor value
   - Timestamp
4. After each write, check file size and truncate if exceeds 2048 bytes
5. Read and display the complete log file contents
6. Report file system statistics and log file information
7. Clean up by removing the log file and unmounting

### Test Structure

- **Test 1**: Initialize log file
- **Test 2**: Write 30 log entries with automatic size checking
- **Test 3**: Read and display log file contents
- **Test 4**: Report file system statistics
- **Test 5**: Display log file size and usage percentage
- **Cleanup**: Remove log file and unmount filesystem

### Configuration

The demo uses the following configuration parameters:

```c
#define LOG_FILE_PATH           "/circular.log"
#define LOG_MAX_SIZE            2048    /* Maximum log file size in bytes */
#define LOG_ENTRY_COUNT         30      /* Number of log entries to write */
```

### USART1 Output

USART1 is configured at **115200 baud, 8N1**. The following output is expected:

```
=== LittleFS Circular Log Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64


=== Internal Flash Circular Log Demo ===

[Test 1] Initialize log file
  Log file created: /circular.log

[Test 2] Write 30 log entries
  Progress: 10/30 entries written
  Progress: 20/30 entries written
  Progress: 30/30 entries written
  Test 2 complete: 30 entries written

[Test 3] Read log file contents
  --- Log File Contents ---
[LOG #00] System event: sensor_value=1000, timestamp=0
[LOG #01] System event: sensor_value=1010, timestamp=100
[LOG #02] System event: sensor_value=1020, timestamp=200
[LOG #03] System event: sensor_value=1030, timestamp=300
[LOG #04] System event: sensor_value=1040, timestamp=400
[LOG #05] System event: sensor_value=1050, timestamp=500
[LOG #06] System event: sensor_value=1060, timestamp=600
[LOG #07] System event: sensor_value=1070, timestamp=700
[LOG #08] System event: sensor_value=1080, timestamp=800
[LOG #09] System event: sensor_value=1090, timestamp=900
[LOG #10] System event: sensor_value=1100, timestamp=1000
[LOG #11] System event: sensor_value=1110, timestamp=1100
[LOG #12] System event: sensor_value=1120, timestamp=1200
[LOG #13] System event: sensor_value=1130, timestamp=1300
[LOG #14] System event: sensor_value=1140, timestamp=1400
[LOG #15] System event: sensor_value=1150, timestamp=1500
[LOG #16] System event: sensor_value=1160, timestamp=1600
[LOG #17] System event: sensor_value=1170, timestamp=1700
[LOG #18] System event: sensor_value=1180, timestamp=1800
[LOG #19] System event: sensor_value=1190, timestamp=1900
[LOG #20] System event: sensor_value=1200, timestamp=2000
[LOG #21] System event: sensor_value=1210, timestamp=2100
[LOG #22] System event: sensor_value=1220, timestamp=2200
[LOG #23] System event: sensor_value=1230, timestamp=2300
[LOG #24] System event: sensor_value=1240, timestamp=2400
[LOG #25] System event: sensor_value=1250, timestamp=2500
[LOG #26] System event: sensor_value=1260, timestamp=2600
[LOG #27] System event: sensor_value=1270, timestamp=2700
[LOG #28] System event: sensor_value=1280, timestamp=2800
[LOG #29] System event: sensor_value=1290, timestamp=2900
  --- End of Log File ---
  Total bytes read: 1728

[Test 4] File system statistics
  Disk version: 2.1
  Block size: 1024 bytes
  Block count: 64
  Used blocks: 6
  Available blocks: 58
  Usage: 9.4%

[Test 5] Log file information
  Log file size: 1728 bytes
  Max log size: 2048 bytes
  Usage: 84.4%

[Cleanup] Removing log file

=== Circular Log Demo Summary ===
Log entries written: 30
Errors: 0
Result: PASS

=== Demo Complete ===
```

### Use Cases

This demo is suitable for applications that need:

- Persistent system event logging
- Error tracking and diagnostics
- Sensor data recording
- Audit trails
- Debug information storage

### Hardware Requirements

- APM32F103 development board
- USART1 connection (PA9/PA10) for serial output
- Internal flash area: 0x08010000 - 0x0801FFFF (64KB)

### IDE Support

- MDK-ARM V5.36
- IAR EWARM V8.50.5

***

## 中文

### 概述

本演示展示了如何在 APM32F103 内部闪存上使用 LittleFS 实现循环日志缓冲区。它演示了如何创建日志文件、追加带时间戳的日志条目，以及当文件大小超过最大值时自动截断文件，模拟循环缓冲区的行为。

### 主要特性

- **日志文件创建**：在 LittleFS 上初始化日志文件
- **带时间戳的条目**：追加 30 条包含传感器值和时间戳的日志条目
- **自动截断**：当文件大小超过 LOG\_MAX\_SIZE（2048 字节）时自动截断
- **日志读取**：读取并显示完整的日志文件内容
- **文件系统统计**：报告磁盘版本、块使用情况和可用空间
- **实际应用场景**：演示带有自动大小管理的嵌入式系统日志记录

### 工作原理

1. 在内部闪存上挂载 LittleFS（0x08010000，64KB，块大小 1KB）
2. 在 `/circular.log` 创建日志文件
3. 追加 30 条日志条目，每条包含：
   - 日志条目编号
   - 模拟传感器值
   - 时间戳
4. 每次写入后检查文件大小，超过 2048 字节时截断
5. 读取并显示完整的日志文件内容
6. 报告文件系统统计信息和日志文件信息
7. 清理：删除日志文件并卸载文件系统

### 测试结构

- **测试 1**：初始化日志文件
- **测试 2**：写入 30 条日志条目，带自动大小检查
- **测试 3**：读取并显示日志文件内容
- **测试 4**：报告文件系统统计信息
- **测试 5**：显示日志文件大小和使用百分比
- **清理**：删除日志文件并卸载文件系统

### 配置参数

本演示使用以下配置参数：

```c
#define LOG_FILE_PATH           "/circular.log"
#define LOG_MAX_SIZE            2048    /* 最大日志文件大小（字节） */
#define LOG_ENTRY_COUNT         30      /* 要写入的日志条目数 */
```

### USART1 输出

USART1 配置为 **115200 波特率，8N1**。预期输出如下：

```
=== LittleFS Circular Log Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64

=== Internal Flash Circular Log Demo ===

[Test 1] Initialize log file
  Log file created: /circular.log

[Test 2] Write 30 log entries
  Progress: 10/30 entries written
  Progress: 20/30 entries written
  Progress: 30/30 entries written
  Test 2 complete: 30 entries written

[Test 3] Read log file contents
  --- Log File Contents ---
[LOG #00] System event: sensor_value=1000, timestamp=0
[LOG #01] System event: sensor_value=1010, timestamp=100
[LOG #02] System event: sensor_value=1020, timestamp=200
...
[LOG #29] System event: sensor_value=1290, timestamp=2900
  --- End of Log File ---
  Total bytes read: 1890

[Test 4] File system statistics
  Disk version: 2.0
  Block size: 1024 bytes
  Block count: 64
  Used blocks: 3
  Available blocks: 61
  Usage: 4.7%

[Test 5] Log file information
  Log file size: 1890 bytes
  Max log size: 2048 bytes
  Usage: 92.3%

[Cleanup] Removing log file

=== Circular Log Demo Summary ===
Log entries written: 30
Errors: 0
Result: PASS

=== Demo Complete ===
```

### 应用场景

本演示适用于需要以下功能的应用：

- 持久化系统事件日志记录
- 错误跟踪和诊断
- 传感器数据记录
- 审计跟踪
- 调试信息存储

### 硬件要求

- APM32F103 开发板
- USART1 连接（PA9/PA10）用于串口输出
- 内部闪存区域：0x08010000 - 0x0801FFFF（64KB）

### IDE 支持

- MDK-ARM V5.36
- IAR EWARM V8.50.5

