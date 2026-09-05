# LittleFS Interactive Shell Demo

***

## English

### Overview

This demo provides an interactive command-line shell for the LittleFS file system on the APM32F107 internal flash. Users can perform file system operations through USART1 using Linux-like commands, creating a persistent storage experience where data remains available across resets.

### Key Features

- **Interactive Shell**: Command-line interface with prompt (`lfs> / $ `) and command history
- **File Operations**: Create, read, copy, move, and delete files
- **Directory Operations**: Create directories, list contents, change working directory
- **Content Manipulation**: Write text to files using echo command with overwrite (`>`) or append (`>>`) redirection
- **File System Information**: View file statistics and disk usage
- **Format Command**: Reformat the filesystem (with warning)
- **Persistent Storage**: Data created in one session remains available after reset

### Available Commands

| Command | Description |
|---------|-------------|
| `help` | Show available commands |
| `ls [path]` | List directory contents |
| `cd <path>` | Change directory |
| `pwd` | Print working directory |
| `mkdir <path>` | Create directory |
| `cat <file>` | Display file content |
| `echo <text> > <file>` | Write text to file (overwrite) |
| `echo <text> >> <file>` | Append text to file |
| `rm <file>` | Remove file |
| `mv <old> <new>` | Move/rename file |
| `cp <src> <dst>` | Copy file |
| `stat <file>` | Show file information |
| `df` | Show filesystem information |
| `format` | Format filesystem (WARNING: erases all data) |

### How It Works

1. System initializes USART1 at 115200 baud, 8N1
2. LittleFS is mounted on internal flash (0x08010000, 64KB, block size 1KB)
3. Shell displays welcome message and command prompt
4. User inputs commands via USART1, processed character by character
5. Commands are executed and results displayed immediately
6. File system state persists across resets

### USART1 Output

USART1 is configured at **115200 baud, 8N1**. The following output is expected on startup:

```
=== LittleFS Interactive Shell Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64
Type 'help' for available commands

lfs> / $
```

### Example Usage

```
lfs> / $ help
Available commands:
  help    - Show available commands
  ls      - List directory contents [path]
  cd      - Change directory <path>
  pwd     - Print working directory
  mkdir   - Create directory <path>
  cat     - Display file content <file>
  echo    - Write text: echo <text> > <file> (overwrite) or >> <file> (append)
  rm      - Remove file <file>
  mv      - Move/rename file <old> <new>
  cp      - Copy file <src> <dst>
  stat    - Show file information <file>
  df      - Show filesystem information
  format  - Format filesystem (WARNING: erases all data)

lfs> / $ mkdir /testdir
Directory created: /testdir

lfs> / $ echo Hello World > /test.txt
File written: /test.txt

lfs> / $ cat /test.txt
Hello World

lfs> / $ ls /
[DIR] testdir
[FILE] test.txt (11 bytes)

lfs> / $ cd /testdir
lfs> /testdir $ pwd
/testdir

lfs> /testdir $ cd /
lfs> / $ df
Filesystem information:
  Disk version: 2.1
  Block size: 1024 bytes
  Block count: 64
  Used blocks: 3
  Available blocks: 61
  Usage: 4.7%
```

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

本 Demo 为 APM32F107 内部 Flash 上的 LittleFS 文件系统提供交互式命令行 Shell。用户可以通过 USART1 使用类 Linux 命令执行文件系统操作，创建持久化存储体验，数据在复位后仍然可用。

### 主要特性

- **交互式 Shell**：带命令提示符（`lfs> / $ `）和命令历史的命令行界面
- **文件操作**：创建、读取、复制、移动和删除文件
- **目录操作**：创建目录、列出内容、更改工作目录
- **内容操作**：使用 echo 命令写入文件，支持覆盖（`>`）或追加（`>>`）重定向
- **文件系统信息**：查看文件统计信息和磁盘使用情况
- **格式化命令**：重新格式化文件系统（带警告）
- **持久化存储**：一个会话中创建的数据在复位后仍然可用

### 可用命令

| 命令 | 描述 |
|------|------|
| `help` | 显示可用命令 |
| `ls [path]` | 列出目录内容 |
| `cd <path>` | 更改目录 |
| `pwd` | 打印工作目录 |
| `mkdir <path>` | 创建目录 |
| `cat <file>` | 显示文件内容 |
| `echo <text> > <file>` | 写入文本到文件（覆盖） |
| `echo <text> >> <file>` | 追加文本到文件 |
| `rm <file>` | 删除文件 |
| `mv <old> <new>` | 移动/重命名文件 |
| `cp <src> <dst>` | 复制文件 |
| `stat <file>` | 显示文件信息 |
| `df` | 显示文件系统信息 |
| `format` | 格式化文件系统（警告：删除所有数据） |

### 工作流程

1. 系统初始化 USART1，波特率 115200，8N1
2. LittleFS 挂载到内部 Flash（0x08010000，64KB，块大小 1KB）
3. Shell 显示欢迎信息和命令提示符
4. 用户通过 USART1 输入命令，逐字符处理
5. 命令执行并立即显示结果
6. 文件系统状态在复位后持久化

### USART1 输出

USART1 配置为 **115200 波特率，8N1**。启动时预期输出如下：

```
=== LittleFS Interactive Shell Demo ===
Flash Area: 0x08010000 - 0x0801FFFF (64KB)
Block Size: 1KB, Block Count: 64
Type 'help' for available commands

lfs> / $
```

### 使用示例

```
lfs> / $ help
Available commands:
  help    - Show available commands
  ls      - List directory contents [path]
  cd      - Change directory <path>
  pwd     - Print working directory
  mkdir   - Create directory <path>
  cat     - Display file content <file>
  echo    - Write text: echo <text> > <file> (overwrite) or >> <file> (append)
  rm      - Remove file <file>
  mv      - Move/rename file <old> <new>
  cp      - Copy file <src> <dst>
  stat    - Show file information <file>
  df      - Show filesystem information
  format  - Format filesystem (WARNING: erases all data)

lfs> / $ mkdir /testdir
Directory created: /testdir

lfs> / $ echo Hello World > /test.txt
File written: /test.txt

lfs> / $ cat /test.txt
Hello World

lfs> / $ ls /
[DIR] testdir
[FILE] test.txt (11 bytes)

lfs> / $ cd /testdir
lfs> /testdir $ pwd
/testdir

lfs> /testdir $ cd /
lfs> / $ df
Filesystem information:
  Disk version: 2.1
  Block size: 1024 bytes
  Block count: 64
  Used blocks: 3
  Available blocks: 61
  Usage: 4.7%
```

### 硬件需求

- APM32F107 MINI 开发板
- USART1 TX 引脚（PA9）连接 USB 转串口模块
- 内部 Flash 区域：0x08010000 - 0x0801FFFF（64KB，块大小 1KB）

### IDE 支持

- MDK-ARM V5.36
- EWARM V8.50.5.26295
- Eclipse V2022-06（4.24.0）
