# LittleFS Multi-File & Directory Demo

## English

### Overview

This demo demonstrates the multi-file and directory structure capabilities of LittleFS on APM32F103 internal flash. It shows how to create hierarchical directories, manage multiple files simultaneously, and perform various file system operations.

### Key Features

- **Multi-level directory structure creation**: `/data`, `/config`, `/data/logs`, `/data/sensors`
- **Multiple file operations**: create, read, write, rename, move, delete
- **Directory traversal and listing**: Browse directory contents at different levels
- **File content verification**: Pattern matching to ensure data integrity
- **File system statistics**: Block usage, available space, and file system information
- **Complete cleanup**: All test files and directories are removed after testing

### Directory Structure Created

```
/
├── data/
│   ├── logs/
│   │   └── system.log
│   └── sensors/
│       ├── sensor_0.dat
│       ├── sensor_1.dat
│       ├── sensor_2.dat
│       ├── sensor_3.dat
│       └── sensor_4.dat
└── config/
    ├── settings.cfg
    └── sensor_backup.dat (moved from sensors)
```

### Test Procedure

1. **Test 1**: Create multi-level directory structure
   - Creates `/data`, `/config`, `/data/logs`, `/data/sensors`

2. **Test 2**: Open and write multiple files
   - Writes configuration file (`/config/settings.cfg`)
   - Writes 5 sensor data files (`/data/sensors/sensor_0.dat` to `sensor_4.dat`)
   - Writes log file (`/data/logs/system.log`)

3. **Test 3**: Directory traversal
   - Lists contents of root directory
   - Lists contents of `/data`, `/data/sensors`, and `/config`

4. **Test 4**: Read and verify files
   - Reads and displays configuration file content
   - Verifies sensor data files with pattern matching

5. **Test 5**: File rename
   - Renames `/data/logs/system.log` to `/data/logs/system_old.log`

6. **Test 6**: File move
   - Moves `/data/sensors/sensor_0.dat` to `/config/sensor_backup.dat`

7. **Test 7**: File delete
   - Deletes `/data/logs/system_old.log`

8. **Test 8**: Final directory listing
   - Shows final directory structure after all operations

9. **Test 9**: File system statistics
   - Displays disk version, block size, block count, usage statistics

### USART1 Output

USART1 is configured at **115200 baud, 8N1**. Expected output:

```
=== LittleFS Multi-File & Directory Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64


=== Internal Flash Multi-File & Directory Demo ===

[Test 1] Create multi-level directory structure
  Created: /data
  Created: /config
  Created: /data/logs
  Created: /data/sensors

[Test 2] Open and write multiple files
  Written: /config/settings.cfg (45 bytes)
  Written: /data/sensors/sensor_0.dat (256 bytes)
  Written: /data/sensors/sensor_1.dat (256 bytes)
  Written: /data/sensors/sensor_2.dat (256 bytes)
  Written: /data/sensors/sensor_3.dat (256 bytes)
  Written: /data/sensors/sensor_4.dat (256 bytes)
  Written: /data/logs/system.log (63 bytes)

[Test 3] Directory traversal
  Root directory (/):
    [DIR] data
    [DIR] config

  /data directory:
    [DIR] logs
    [DIR] sensors

  /data/sensors directory:
    [FILE] sensor_0.dat (256 bytes)
    [FILE] sensor_1.dat (256 bytes)
    [FILE] sensor_2.dat (256 bytes)
    [FILE] sensor_3.dat (256 bytes)
    [FILE] sensor_4.dat (256 bytes)

  /config directory:
    [FILE] settings.cfg (45 bytes)

[Test 4] Read and verify files
  Read /config/settings.cfg:
    device_id=APM32F103
    version=1.0
    mode=normal

  Verified: /data/sensors/sensor_0.dat (pattern OK)
  Verified: /data/sensors/sensor_1.dat (pattern OK)
  Verified: /data/sensors/sensor_2.dat (pattern OK)
  Verified: /data/sensors/sensor_3.dat (pattern OK)
  Verified: /data/sensors/sensor_4.dat (pattern OK)

[Test 5] File rename
  Renamed: /data/logs/system.log -> /data/logs/system_old.log
  Verified: /data/logs/system_old.log exists

[Test 6] File move
  Moved: /data/sensors/sensor_0.dat -> /config/sensor_backup.dat
  Verified: /config/sensor_backup.dat exists

[Test 7] File delete
  Deleted: /data/logs/system_old.log
  Verified: /data/logs/system_old.log no longer exists

[Test 8] Final directory structure
  Root (/):
    [DIR] data
    [DIR] config

  /data:
    [DIR] logs
    [DIR] sensors

  /data/sensors:
    [FILE] sensor_1.dat (256 bytes)
    [FILE] sensor_2.dat (256 bytes)
    [FILE] sensor_3.dat (256 bytes)
    [FILE] sensor_4.dat (256 bytes)

  /config:
    [FILE] settings.cfg (45 bytes)
    [FILE] sensor_backup.dat (256 bytes)

[Test 9] File system statistics
  Disk version: 2.0
  Block size: 1024 bytes
  Block count: 64
  Max file name: 255 bytes
  Max file size: 2147483647 bytes
  Used blocks: 8
  Available blocks: 56
  Usage: 12.5%

[Cleanup] Removing test files and directories

=== Multi-File Demo Summary ===
Directories created: 4
Files created: 7
Files renamed: 1
Files moved: 1
Files deleted: 1
Errors: 0
Result: PASS

=== Demo Complete ===
```

### Use Cases

This demo is suitable for applications that need:
- Hierarchical file organization
- Configuration file management
- Data logging with multiple data sources
- File backup and migration
- Dynamic file system management

### Hardware Requirements

- APM32F103 development board
- USART1 connection (PA9/PA10) for serial output
- Internal flash area: 0x08010000 - 0x0801FFFF (64KB)

### IDE Support

- MDK-ARM V5.36
- IAR EWARM V8.50.5
- Eclipse V2022-06 (4.24.0)

---

## 中文

### 概述

本演示展示了 LittleFS 在 APM32F103 内部闪存上的多文件和目录结构功能。它展示了如何创建分层目录、同时管理多个文件以及执行各种文件系统操作。

### 主要特性

- **多级目录结构创建**：`/data`、`/config`、`/data/logs`、`/data/sensors`
- **多种文件操作**：创建、读取、写入、重命名、移动、删除
- **目录遍历和列表**：浏览不同层级的目录内容
- **文件内容验证**：模式匹配确保数据完整性
- **文件系统统计**：块使用情况、可用空间和文件系统信息
- **完整清理**：测试后删除所有测试文件和目录

### 创建的目录结构

```
/
├── data/
│   ├── logs/
│   │   └── system.log
│   └── sensors/
│       ├── sensor_0.dat
│       ├── sensor_1.dat
│       ├── sensor_2.dat
│       ├── sensor_3.dat
│       └── sensor_4.dat
└── config/
    ├── settings.cfg
    └── sensor_backup.dat (从 sensors 移动)
```

### 测试流程

1. **测试 1**：创建多级目录结构
   - 创建 `/data`、`/config`、`/data/logs`、`/data/sensors`

2. **测试 2**：打开并写入多个文件
   - 写入配置文件 (`/config/settings.cfg`)
   - 写入 5 个传感器数据文件 (`/data/sensors/sensor_0.dat` 到 `sensor_4.dat`)
   - 写入日志文件 (`/data/logs/system.log`)

3. **测试 3**：目录遍历
   - 列出根目录内容
   - 列出 `/data`、`/data/sensors` 和 `/config` 的内容

4. **测试 4**：读取并验证文件
   - 读取并显示配置文件内容
   - 使用模式匹配验证传感器数据文件

5. **测试 5**：文件重命名
   - 将 `/data/logs/system.log` 重命名为 `/data/logs/system_old.log`

6. **测试 6**：文件移动
   - 将 `/data/sensors/sensor_0.dat` 移动到 `/config/sensor_backup.dat`

7. **测试 7**：文件删除
   - 删除 `/data/logs/system_old.log`

8. **测试 8**：最终目录列表
   - 显示所有操作后的最终目录结构

9. **测试 9**：文件系统统计
   - 显示磁盘版本、块大小、块数量、使用统计

### USART1 输出

USART1 配置为 **115200 波特率，8N1**。预期输出：

```
=== LittleFS Multi-File & Directory Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64


=== Internal Flash Multi-File & Directory Demo ===

[Test 1] Create multi-level directory structure
  Created: /data
  Created: /config
  Created: /data/logs
  Created: /data/sensors

[Test 2] Open and write multiple files
  Written: /config/settings.cfg (45 bytes)
  Written: /data/sensors/sensor_0.dat (256 bytes)
  Written: /data/sensors/sensor_1.dat (256 bytes)
  Written: /data/sensors/sensor_2.dat (256 bytes)
  Written: /data/sensors/sensor_3.dat (256 bytes)
  Written: /data/sensors/sensor_4.dat (256 bytes)
  Written: /data/logs/system.log (63 bytes)

[Test 3] Directory traversal
  Root directory (/):
    [DIR] data
    [DIR] config

  /data directory:
    [DIR] logs
    [DIR] sensors

  /data/sensors directory:
    [FILE] sensor_0.dat (256 bytes)
    [FILE] sensor_1.dat (256 bytes)
    [FILE] sensor_2.dat (256 bytes)
    [FILE] sensor_3.dat (256 bytes)
    [FILE] sensor_4.dat (256 bytes)

  /config directory:
    [FILE] settings.cfg (45 bytes)

[Test 4] Read and verify files
  Read /config/settings.cfg:
    device_id=APM32F103
    version=1.0
    mode=normal

  Verified: /data/sensors/sensor_0.dat (pattern OK)
  Verified: /data/sensors/sensor_1.dat (pattern OK)
  Verified: /data/sensors/sensor_2.dat (pattern OK)
  Verified: /data/sensors/sensor_3.dat (pattern OK)
  Verified: /data/sensors/sensor_4.dat (pattern OK)

[Test 5] File rename
  Renamed: /data/logs/system.log -> /data/logs/system_old.log
  Verified: /data/logs/system_old.log exists

[Test 6] File move
  Moved: /data/sensors/sensor_0.dat -> /config/sensor_backup.dat
  Verified: /config/sensor_backup.dat exists

[Test 7] File delete
  Deleted: /data/logs/system_old.log
  Verified: /data/logs/system_old.log no longer exists

[Test 8] Final directory structure
  Root (/):
    [DIR] data
    [DIR] config

  /data:
    [DIR] logs
    [DIR] sensors

  /data/sensors:
    [FILE] sensor_1.dat (256 bytes)
    [FILE] sensor_2.dat (256 bytes)
    [FILE] sensor_3.dat (256 bytes)
    [FILE] sensor_4.dat (256 bytes)

  /config:
    [FILE] settings.cfg (45 bytes)
    [FILE] sensor_backup.dat (256 bytes)

[Test 9] File system statistics
  Disk version: 2.0
  Block size: 1024 bytes
  Block count: 64
  Max file name: 255 bytes
  Max file size: 2147483647 bytes
  Used blocks: 8
  Available blocks: 56
  Usage: 12.5%

[Cleanup] Removing test files and directories

=== Multi-File Demo Summary ===
Directories created: 4
Files created: 7
Files renamed: 1
Files moved: 1
Files deleted: 1
Errors: 0
Result: PASS

=== Demo Complete ===
```

### 应用场景

本演示适用于需要以下功能的应用：
- 分层文件组织
- 配置文件管理
- 多数据源的数据日志记录
- 文件备份和迁移
- 动态文件系统管理

### 硬件要求

- APM32F103 开发板
- USART1 连接 (PA9/PA10) 用于串口输出
- 内部闪存区域：0x08010000 - 0x0801FFFF (64KB)

### IDE 支持

- MDK-ARM V5.36
- IAR EWARM V8.50.5
- Eclipse V2022-06 (4.24.0)
