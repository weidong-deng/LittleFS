/*!
 * @file        readme.txt
 *
 * @brief       This file is routine instruction
 *
 * @version     V1.0.0
 *
 * @date        2026-08-16
 *
 * @attention
 *
 *  Copyright (C) 2020-2022 Geehy Semiconductor
 *
 *  You may not use this file except in compliance with the
 *  GEEHY COPYRIGHT NOTICE (GEEHY SOFTWARE PACKAGE LICENSE).
 *
 *  The program is only for reference, which is distributed in the hope
 *  that it will be useful and instructional for customers to develop
 *  their software. Unless required by applicable law or agreed to in
 *  writing, the program is distributed on an "AS IS" BASIS, WITHOUT
 *  ANY WARRANTY OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the GEEHY SOFTWARE PACKAGE LICENSE for the governing permissions
 *  and limitations under the License.
 */


&par Example Description

  This demo demonstrates the multi-file and directory structure
  capabilities of LittleFS on APM32F107 internal flash. It shows
  how to create hierarchical directories, manage multiple files
  simultaneously, and perform various file system operations.

  Key features:
  - Multi-level directory structure creation (/data, /config, /data/logs, /data/sensors)
  - Multiple file operations (create, read, write, rename, move, delete)
  - Directory traversal and listing
  - File content verification with pattern matching
  - File system statistics and usage reporting
  - Complete cleanup of test files and directories

  USART1 is used for output at 115200 baud, 8N1.

  The demo creates a realistic directory structure with configuration
  files, sensor data, and log files, demonstrating typical embedded
  file system usage patterns.

&par Directory contents

  - Source/main.c                  Main program
  - Source/system_apm32f10x.c      System clock configuration
  - Source/apm32f10x_int.c         Interrupt handlers
  - Include/main.h                 Main header
  - Include/apm32f10x_int.h        Interrupt handlers header

&par IDE environment

  - MDK-ARM V5.36
  - EWARM V8.50.5.26295
  - Eclipse V2022-06（4.24.0）

&par Hardware and Software environment

  - This example runs on APM32F107 MINI Devices.
  - USART1 TX: PA9  (115200 baud, 8N1)
  - Internal flash area: 0x08010000 - 0x0801FFFF (64KB)
