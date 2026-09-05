# LittleFS Wear Leveling Demo

***

## English

### Overview

This demo demonstrates the wear leveling capability of the LittleFS file system on the APM32F103 internal flash. By performing intensive and repeated write operations, it verifies that LittleFS distributes erase cycles evenly across all flash blocks, preventing premature wear-out of any single block.

### Key Features

- **Repeated Write Stress Test**: 1000 iterations of writing 512 bytes to the same file with read-back verification, creating heavy write pressure on a small flash area.
- **Multi-file Write Test**: Writes to 10 separate files to verify wear spreading across the file system.
- **Directory Operations**: Creates a directory with 5 files and lists its contents.
- **File System Statistics**: Reports disk version, block size, block count, usage, and available space.
- **Garbage Collection**: Triggers and verifies GC operation.
- **File Rename**: Tests the file rename operation with existence verification.
- **Block Erase Statistics**: Tracks per-block erase counts and calculates the wear distribution ratio to quantify wear leveling effectiveness.

### How It Works

1. Mount LittleFS on internal flash (0x08010000, 64KB, block size 1KB).
2. Reset the block erase statistics counters.
3. **Test 1** - Repeatedly write 512 bytes to `/wear_test.dat` 1000 times. Each iteration generates a unique data pattern, writes it, then reads it back for verification. Progress is printed every 100 iterations.
4. **Test 2** - Write 512 bytes to each of 10 files (`/multi_0.dat` to `/multi_9.dat`).
5. **Test 3** - Create directory `/testdir` with 5 files inside, then list its contents.
6. **Test 4** - Query and display file system statistics (version, block info, usage).
7. **Test 5** - Trigger garbage collection and report used blocks before/after.
8. **Test 6** - Rename `/wear_test.dat` to `/wear_test_renamed.dat` and verify.
9. **Cleanup** - Remove all test files, directory, and unmount.
10. **Summary** - Print total writes, errors, and block erase statistics with wear distribution analysis.

### Wear Leveling Verification

The core purpose of this demo is to verify wear leveling. After the test, the block erase statistics are printed. Key metrics:

- **Total erases**: Sum of all block erase operations.
- **Min/Max erases**: The least and most erased blocks.
- **Wear distribution ratio**: `Max erases / Min erases`. A lower ratio indicates better wear leveling. For data blocks (blocks 4-63), the ratio should be close to 1:1.

### USART1 Output

USART1 is configured at **115200 baud, 8N1**. The following output is expected:

```
=== LittleFS Internal Flash Wear Leveling Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64


=== Internal Flash wear leveling ===
Start writing tests, iterating 1000 times...

[Test 1] Repeated write to same file
  Progress: 100/1000 iterations
  Progress: 200/1000 iterations
  Progress: 300/1000 iterations
  Progress: 400/1000 iterations
  Progress: 500/1000 iterations
  Progress: 600/1000 iterations
  Progress: 700/1000 iterations
  Progress: 800/1000 iterations
  Progress: 900/1000 iterations
  Progress: 1000/1000 iterations
  Test 1 complete: 1000 writes, 0 errors

[Test 2] Write to multiple files
  Test 2 complete: 10 files written

[Test 3] Directory operations
  Created directory: /testdir
  Directory contents:
  [DIR] .
  [DIR] ..
  file_0.dat (file)
  file_1.dat (file)
  file_2.dat (file)
  file_3.dat (file)
  file_4.dat (file)

[Test 4] File system statistics
  Disk version: 2.1
  Block size: 1024 (0x400) bytes
  Block count: 64 (0x40)
  [Config] Max file name: 255 bytes
  [Config] Max file size: 2147483647 (0x7FFFFFFF) bytes
  [Actual] Used blocks: 22 (0x16)
  [Actual] Available blocks: 42 (0x2A)
  [Actual] Usage: 34.4%

[Test 5] Garbage collection
  GC completed successfully
  Used blocks after GC: 22

[Test 6] File rename
  Renamed /wear_test.dat -> /wear_test_renamed.dat
  Verified: renamed file exists

[Cleanup] Removing test files

=== Wear Leveling Test Summary ===
Total iterations: 1000
Total writes: 1010
Errors: 0
Result: PASS

=== Block Erase Statistics ===
Block erase counts (block:count):
  [ 0]:  1  [ 2]: 18  [ 3]: 18  [ 4]: 18  [ 5]: 18  [ 6]: 18  [ 7]: 18
  [ 8]: 18  [ 9]: 19  [10]: 19  [11]: 18  [12]: 18  [13]: 18  [14]: 70  [15]: 18
  [16]: 17  [17]: 17  [18]: 17  [19]: 17  [20]: 17  [21]:132  [22]: 17  [23]: 17
  [24]: 17  [25]: 17  [26]: 78  [27]: 17  [28]: 17  [29]: 17  [30]: 17  [31]: 17
  [32]: 17  [33]: 17  [34]: 17  [35]: 17  [36]: 17  [37]: 17  [38]: 17  [39]: 17
  [40]: 17  [41]: 17  [42]: 17  [43]: 17  [44]: 17  [45]: 17  [46]: 17  [47]: 17
  [48]: 17  [49]: 17  [50]: 17  [51]: 17  [52]: 17  [53]: 17  [54]: 17  [55]: 17
  [56]: 17  [57]: 17  [58]: 17  [59]: 17  [60]: 17  [61]: 17  [62]:  9  [63]:  9

Summary:
  Total blocks: 64
  Blocks with erases: 63
  Total erases: 1283
  Min erases: 1
  Max erases: 132
  Overall wear distribution ratio: 132.00

Data Blocks Summary (blocks 4-63):
  Data blocks with erases: 60
  Data block total erases: 1246
  Data block min erases: 9
  Data block max erases: 132
  Data block wear distribution ratio: 14.67

=== Test Complete ===
```

### Interpreting the Results

- **Test 1-3**: All file and directory operations succeed with 0 errors, confirming basic file system functionality under heavy write load.
- **Test 4**: Shows the file system is using 22 of 64 blocks (34.4%) after the stress test.
- **Test 5**: GC completes successfully, used blocks remain at 22.
- **Test 6**: File rename works correctly.
- **Block Erase Statistics**: The data block wear distribution ratio of 14.67 (max 132 / min 9) shows that while some blocks receive more erases due to LittleFS metadata management, the wear is spread across all 60 data blocks rather than concentrated on a few. The overall ratio is higher because block 0 (used for bootloader/superblock) is only erased once.

### Hardware Requirements

- APM32F103 MINI board
- USART1 TX pin (PA9) connected to USB-to-UART adapter
- Internal flash area: 0x08010000 - 0x0801FFFF (64KB, block size 1KB)

### IDE Support

- MDK-ARM V5.36
- EWARM V8.50.5.26295
- Eclipse V2022-06（4.24.0）

***

## 中文

### 概述

本 Demo 演示了 LittleFS 文件系统在 APM32F103 内部 Flash 上的磨损均衡能力。通过密集重复写入操作，验证 LittleFS 能够将擦除次数均匀分布到所有 Flash 块上，避免单个块过早磨损。

### 主要特性

- **重复写入压力测试**：对同一文件进行 1000 次 512 字节写入，每次写入后回读验证，在小面积 Flash 上产生大量写入压力。
- **多文件写入测试**：向 10 个独立文件写入数据，验证文件系统的磨损分散能力。
- **目录操作**：创建包含 5 个文件的目录并列出内容。
- **文件系统统计**：报告磁盘版本、块大小、块数量、使用率和可用空间。
- **垃圾回收**：触发并验证 GC 操作。
- **文件重命名**：测试文件重命名操作并验证。
- **块擦除统计**：跟踪每个块的擦除次数，计算磨损分布比率以量化磨损均衡效果。

### 工作流程

1. 在内部 Flash（0x08010000，64KB，块大小 1KB）上挂载 LittleFS。
2. 复位块擦除统计计数器。
3. **测试 1** - 对 `/wear_test.dat` 重复写入 512 字节共 1000 次。每次迭代生成唯一数据模式，写入后回读验证。每 100 次迭代打印进度。
4. **测试 2** - 向 10 个文件（`/multi_0.dat` 到 `/multi_9.dat`）各写入 512 字节。
5. **测试 3** - 创建目录 `/testdir`，内含 5 个文件，然后列出目录内容。
6. **测试 4** - 查询并显示文件系统统计信息（版本、块信息、使用率）。
7. **测试 5** - 触发垃圾回收，报告前后使用的块数量。
8. **测试 6** - 将 `/wear_test.dat` 重命名为 `/wear_test_renamed.dat` 并验证。
9. **清理** - 删除所有测试文件、目录，卸载文件系统。
10. **摘要** - 打印总写入次数、错误数和块擦除统计，含磨损分布分析。

### 磨损均衡验证

本 Demo 的核心目的是验证磨损均衡。测试结束后打印块擦除统计，关键指标：

- **总擦除次数**：所有块擦除操作的总和。
- **最小/最大擦除次数**：擦除次数最少和最多的块。
- **磨损分布比率**：`最大擦除次数 / 最小擦除次数`。比率越低表示磨损均衡效果越好。对于数据块（块 4-63），比率应接近 1:1。

### USART1 输出

USART1 配置为 **115200 波特率，8N1**。预期输出如下：

```
=== LittleFS Internal Flash Wear Leveling Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64


=== Internal Flash wear leveling ===
Start writing tests, iterating 1000 times...

[Test 1] Repeated write to same file
  Progress: 100/1000 iterations
  Progress: 200/1000 iterations
  Progress: 300/1000 iterations
  Progress: 400/1000 iterations
  Progress: 500/1000 iterations
  Progress: 600/1000 iterations
  Progress: 700/1000 iterations
  Progress: 800/1000 iterations
  Progress: 900/1000 iterations
  Progress: 1000/1000 iterations
  Test 1 complete: 1000 writes, 0 errors

[Test 2] Write to multiple files
  Test 2 complete: 10 files written

[Test 3] Directory operations
  Created directory: /testdir
  Directory contents:
  [DIR] .
  [DIR] ..
  file_0.dat (file)
  file_1.dat (file)
  file_2.dat (file)
  file_3.dat (file)
  file_4.dat (file)

[Test 4] File system statistics
  Disk version: 2.1
  Block size: 1024 (0x400) bytes
  Block count: 64 (0x40)
  [Config] Max file name: 255 bytes
  [Config] Max file size: 2147483647 (0x7FFFFFFF) bytes
  [Actual] Used blocks: 22 (0x16)
  [Actual] Available blocks: 42 (0x2A)
  [Actual] Usage: 34.4%

[Test 5] Garbage collection
  GC completed successfully
  Used blocks after GC: 22

[Test 6] File rename
  Renamed /wear_test.dat -> /wear_test_renamed.dat
  Verified: renamed file exists

[Cleanup] Removing test files

=== Wear Leveling Test Summary ===
Total iterations: 1000
Total writes: 1010
Errors: 0
Result: PASS

=== Block Erase Statistics ===
Block erase counts (block:count):
  [ 0]:  1  [ 2]: 18  [ 3]: 18  [ 4]: 18  [ 5]: 18  [ 6]: 18  [ 7]: 18
  [ 8]: 18  [ 9]: 19  [10]: 19  [11]: 18  [12]: 18  [13]: 18  [14]: 70  [15]: 18
  [16]: 17  [17]: 17  [18]: 17  [19]: 17  [20]: 17  [21]:132  [22]: 17  [23]: 17
  [24]: 17  [25]: 17  [26]: 78  [27]: 17  [28]: 17  [29]: 17  [30]: 17  [31]: 17
  [32]: 17  [33]: 17  [34]: 17  [35]: 17  [36]: 17  [37]: 17  [38]: 17  [39]: 17
  [40]: 17  [41]: 17  [42]: 17  [43]: 17  [44]: 17  [45]: 17  [46]: 17  [47]: 17
  [48]: 17  [49]: 17  [50]: 17  [51]: 17  [52]: 17  [53]: 17  [54]: 17  [55]: 17
  [56]: 17  [57]: 17  [58]: 17  [59]: 17  [60]: 17  [61]: 17  [62]:  9  [63]:  9

Summary:
  Total blocks: 64
  Blocks with erases: 63
  Total erases: 1283
  Min erases: 1
  Max erases: 132
  Overall wear distribution ratio: 132.00

Data Blocks Summary (blocks 4-63):
  Data blocks with erases: 60
  Data block total erases: 1246
  Data block min erases: 9
  Data block max erases: 132
  Data block wear distribution ratio: 14.67

=== Test Complete ===
```

### 结果分析

- **测试 1-3**：所有文件和目录操作均成功，0 错误，确认了在大量写入负载下文件系统的基本功能正常。
- **测试 4**：压力测试后文件系统使用了 64 个块中的 22 个（34.4%）。
- **测试 5**：GC 成功完成，使用块数保持为 22。
- **测试 6**：文件重命名正常工作。
- **块擦除统计**：数据块磨损分布比率为 14.67（最大 132 / 最小 9），表明虽然由于 LittleFS 元数据管理，部分块的擦除次数较多，但磨损已分散到全部 60 个数据块上，而非集中在少数块。整体比率较高是因为块 0（用于引导加载程序/超级块）仅被擦除 1 次。

### 硬件需求

- APM32F103 MINI 开发板
- USART1 TX 引脚（PA9）连接 USB 转串口模块
- 内部 Flash 区域：0x08010000 - 0x0801FFFF（64KB，块大小 1KB）

### IDE 支持

- MDK-ARM V5.36
- EWARM V8.50.5.26295

