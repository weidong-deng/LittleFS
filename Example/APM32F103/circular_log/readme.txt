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

  This demo demonstrates how to implement a circular log buffer using
  LittleFS on APM32F103 internal flash. It shows how to create a log file,
  append log entries with timestamps, and automatically truncate the file
  when it exceeds a maximum size, simulating circular buffer behavior.

  Key features:
  - Create and initialize a log file
  - Append timestamped log entries (30 entries with sensor values)
  - Automatic file truncation when size exceeds LOG_MAX_SIZE (2048 bytes)
  - Read and display complete log file contents
  - File system statistics and usage reporting
  - Demonstrates practical use case for embedded system logging

  USART1 is used for output at 115200 baud, 8N1.

  This demo is suitable for applications that need persistent logging
  with automatic size management, such as system event logging, error
  tracking, or sensor data recording.

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

  - This example runs on APM32F103 MINI Devices.
  - USART1 TX: PA9  (115200 baud, 8N1)
  - Internal flash area: 0x08010000 - 0x0801FFFF (64KB)
