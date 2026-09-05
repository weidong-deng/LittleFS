# LittleFS Performance Benchmark Demo

***

## English

### Overview

This demo demonstrates the performance benchmark capabilities of LittleFS on the APM32F107 internal flash. It measures sequential and random read/write speeds, as well as file creation and deletion times, providing comprehensive performance metrics for the file system.

### Key Features

- **Sequential Write Speed Test**: Writes 32KB of data in 512-byte chunks, measuring throughput in KB/s.
- **Sequential Read Speed Test**: Reads 32KB of data in 512-byte chunks, measuring throughput in KB/s.
- **Random Write Speed Test**: Performs 100 random seek and write operations of 64 bytes each, measuring throughput and average operation time.
- **Random Read Speed Test**: Performs 100 random seek and read operations of 64 bytes each, measuring throughput and average operation time.
- **File Creation Speed Test**: Creates 20 files sequentially, measuring average time per file.
- **File Deletion Speed Test**: Deletes 20 files sequentially, measuring average time per file.
- **Performance Summary**: Displays all benchmark results in a consolidated summary.

### How It Works

1. Mount LittleFS on internal flash (0x08010000, 64KB, block size 1KB).
2. **Test 1** - Sequential write: Open `/benchmark.dat`, write 32KB in 512-byte chunks, measure time and calculate throughput.
3. **Test 2** - Sequential read: Read back the 32KB file in 512-byte chunks, measure time and calculate throughput.
4. **Test 3** - Random write: Perform 100 random seek operations within the file, write 64 bytes at each position, measure total time and average per operation.
5. **Test 4** - Random read: Perform 100 random seek operations, read 64 bytes at each position, measure total time and average per operation.
6. **Test 5** - File creation/deletion: Create 20 files (`/test_0.dat` to `/test_19.dat`) with 64 bytes each, then delete them all, measuring time for each operation.
7. **Cleanup** - Remove the benchmark file and unmount.
8. **Summary** - Print all performance metrics.

### Performance Metrics

The demo reports the following metrics:

- **Sequential Write**: Throughput in KB/s (32KB total)
- **Sequential Read**: Throughput in KB/s (32KB total)
- **Random Write**: Throughput in KB/s and average time per operation in microseconds
- **Random Read**: Throughput in KB/s and average time per operation in microseconds
- **File Create**: Average time per file in milliseconds
- **File Delete**: Average time per file in milliseconds

### USART1 Output

USART1 is configured at **115200 baud, 8N1**. The following output is expected:

```
=== LittleFS Performance Benchmark Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64


=== Starting Benchmark Tests ===
Filesystem mounted successfully

[Test 1] Sequential Write Speed
  Writing 32768 bytes in 512-byte chunks...
  Completed in 450 ms
  Throughput: 71.12 KB/s

[Test 2] Sequential Read Speed
  Reading 32768 bytes in 512-byte chunks...
  Completed in 120 ms
  Throughput: 266.66 KB/s

[Test 3] Random Write Speed
  Performing 100 random writes of 64 bytes...
  Completed in 380 ms
  Throughput: 16.42 KB/s
  Avg time per operation: 3800 us

[Test 4] Random Read Speed
  Performing 100 random reads of 64 bytes...
  Completed in 95 ms
  Throughput: 65.68 KB/s
  Avg time per operation: 950 us

[Test 5] File Create/Delete Speed
  Creating 20 files...
  Created 20 files in 280 ms
  Avg time per file: 14.00 ms
  Deleting 20 files...
  Deleted 20 files in 150 ms
  Avg time per file: 7.50 ms

=== Benchmark Summary ===
Sequential Write: 71.12 KB/s
Sequential Read: 266.66 KB/s
Random Write: 16.42 KB/s
Random Read: 65.68 KB/s
File Create: 14.00 ms/file
File Delete: 7.50 ms/file
Errors: 0
Result: PASS

=== Benchmark Complete ===
```

### Interpreting the Results

- **Sequential operations** are typically faster than random operations due to better cache utilization and fewer seek operations.
- **Read operations** are generally faster than write operations because flash writes require erase-before-write cycles.
- **File creation** takes longer than deletion because creation involves allocating blocks and writing metadata, while deletion only marks blocks as free.
- The actual numbers will vary based on flash characteristics, system clock, and LittleFS configuration.

### Hardware Requirements

- APM32F107 MINI board
- USART1 TX pin (PA9) connected to USB-to-UART adapter
- Internal flash area: 0x08010000 - 0x0801FFFF (64KB, block size 1KB)

### IDE Support

- MDK-ARM V5.36
- EWARM V8.50.5.26295
- Eclipse V2022-06（4.24.0）

***

## 中文

### 概述

本 Demo 演示了 LittleFS 文件系统在 APM32F107 内部 Flash 上的性能基准测试能力。它测量顺序和随机读写速度，以及文件创建和删除时间，为文件系统提供全面的性能指标。

### 主要特性

- **顺序写入速度测试**：以 512 字节块写入 32KB 数据，测量吞吐量（KB/s）。
- **顺序读取速度测试**：以 512 字节块读取 32KB 数据，测量吞吐量（KB/s）。
- **随机写入速度测试**：执行 100 次随机寻址和写入操作，每次 64 字节，测量吞吐量和平均操作时间。
- **随机读取速度测试**：执行 100 次随机寻址和读取操作，每次 64 字节，测量吞吐量和平均操作时间。
- **文件创建速度测试**：顺序创建 20 个文件，测量每个文件的平均时间。
- **文件删除速度测试**：顺序删除 20 个文件，测量每个文件的平均时间。
- **性能摘要**：在整合摘要中显示所有基准测试结果。

### 工作流程

1. 在内部 Flash（0x08010000，64KB，块大小 1KB）上挂载 LittleFS。
2. **测试 1** - 顺序写入：打开 `/benchmark.dat`，以 512 字节块写入 32KB，测量时间并计算吞吐量。
3. **测试 2** - 顺序读取：以 512 字节块读回 32KB 文件，测量时间并计算吞吐量。
4. **测试 3** - 随机写入：在文件内执行 100 次随机寻址操作，每个位置写入 64 字节，测量总时间和每次操作的平均时间。
5. **测试 4** - 随机读取：执行 100 次随机寻址操作，每个位置读取 64 字节，测量总时间和每次操作的平均时间。
6. **测试 5** - 文件创建/删除：创建 20 个文件（`/test_0.dat` 到 `/test_19.dat`），每个 64 字节，然后全部删除，测量每次操作的时间。
7. **清理** - 删除基准测试文件并卸载。
8. **摘要** - 打印所有性能指标。

### 性能指标

Demo 报告以下指标：

- **顺序写入**：吞吐量（KB/s），总计 32KB
- **顺序读取**：吞吐量（KB/s），总计 32KB
- **随机写入**：吞吐量（KB/s）和每次操作的平均时间（微秒）
- **随机读取**：吞吐量（KB/s）和每次操作的平均时间（微秒）
- **文件创建**：每个文件的平均时间（毫秒）
- **文件删除**：每个文件的平均时间（毫秒）

### USART1 输出

USART1 配置为 **115200 波特率，8N1**。预期输出如下：

```
=== LittleFS Performance Benchmark Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64


=== Starting Benchmark Tests ===
Filesystem mounted successfully

[Test 1] Sequential Write Speed
  Writing 32768 bytes in 512-byte chunks...
  Completed in 450 ms
  Throughput: 71.12 KB/s

[Test 2] Sequential Read Speed
  Reading 32768 bytes in 512-byte chunks...
  Completed in 120 ms
  Throughput: 266.66 KB/s

[Test 3] Random Write Speed
  Performing 100 random writes of 64 bytes...
  Completed in 380 ms
  Throughput: 16.42 KB/s
  Avg time per operation: 3800 us

[Test 4] Random Read Speed
  Performing 100 random reads of 64 bytes...
  Completed in 95 ms
  Throughput: 65.68 KB/s
  Avg time per operation: 950 us

[Test 5] File Create/Delete Speed
  Creating 20 files...
  Created 20 files in 280 ms
  Avg time per file: 14.00 ms
  Deleting 20 files...
  Deleted 20 files in 150 ms
  Avg time per file: 7.50 ms

=== Benchmark Summary ===
Sequential Write: 71.12 KB/s
Sequential Read: 266.66 KB/s
Random Write: 16.42 KB/s
Random Read: 65.68 KB/s
File Create: 14.00 ms/file
File Delete: 7.50 ms/file
Errors: 0
Result: PASS

=== Benchmark Complete ===
```

### 结果分析

- **顺序操作**通常比随机操作更快，因为缓存利用率更高且寻址操作更少。
- **读取操作**通常比写入操作更快，因为 Flash 写入需要擦除后写入的周期。
- **文件创建**比删除耗时更长，因为创建涉及分配块和写入元数据，而删除只需标记块为空闲。
- 实际数值会因 Flash 特性、系统时钟和 LittleFS 配置而异。

### 硬件需求

- APM32F107 MINI 开发板
- USART1 TX 引脚（PA9）连接 USB 转串口模块
- 内部 Flash 区域：0x08010000 - 0x0801FFFF（64KB，块大小 1KB）

### IDE 支持

- MDK-ARM V5.36
- EWARM V8.50.5.26295
- Eclipse V2022-06（4.24.0）
