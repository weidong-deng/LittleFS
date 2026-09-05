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

  This demo provides an interactive command-line shell for LittleFS on
  APM32F103 internal flash. Users can perform file system operations
  through USART1 using Linux-like commands.

  Key features:
  - Interactive shell with command prompt (lfs> / $ )
  - File operations: create, read, copy, move, delete
  - Directory operations: create, list, change directory
  - File content manipulation with echo command (overwrite/append)
  - File system information and statistics
  - Format filesystem command
  - Persistent storage across resets (data preserved)

  Available commands:
  - help    : Show available commands
  - ls      : List directory contents
  - cd      : Change directory
  - pwd     : Print working directory
  - mkdir   : Create directory
  - cat     : Display file content
  - echo    : Write text to file (> overwrite, >> append)
  - rm      : Remove file
  - mv      : Move/rename file
  - cp      : Copy file
  - stat    : Show file information
  - df      : Show filesystem information
  - format  : Format filesystem (WARNING: erases all data)

  USART1 is used for input/output at 115200 baud, 8N1.

  The shell provides a persistent file system experience where data
  created in one session remains available after reset.

&par Directory contents

  - Source/main.c                  Main program with shell initialization
  - Source/system_apm32f10x.c      System clock configuration
  - Source/apm32f10x_int.c         Interrupt handlers
  - Include/main.h                 Main header
  - Include/apm32f10x_int.h        Interrupt handlers header
  - Littlefs/port/shell.c          Shell command implementation
  - Littlefs/port/shell.h          Shell header

&par IDE environment

  - MDK-ARM V5.36
  - EWARM V8.50.5.26295
  - Eclipse V2022-06（4.24.0）

&par Hardware and Software environment

  - This example runs on APM32F103 MINI Devices.
  - USART1 TX: PA9  (115200 baud, 8N1)
  - Internal flash area: 0x08010000 - 0x0801FFFF (64KB)
