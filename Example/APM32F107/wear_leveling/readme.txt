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

  This demo demonstrates the wear leveling capability of LittleFS on
  APM32F107 internal flash. It performs intensive write operations to
  verify that erase counts are evenly distributed across flash blocks,
  preventing premature wear-out of any single block.

  Key features:
  - 1000-iteration repeated write stress test to a single file
  - Multi-file write test to verify wear spreading
  - Directory creation and listing
  - File system statistics and usage reporting
  - Garbage collection test
  - File rename operation
  - Block erase count statistics with wear distribution ratio

  USART1 is used for output at 115200 baud, 8N1.

  After the test completes, the block erase statistics show how evenly
  erases are distributed across all flash blocks, demonstrating the
  effectiveness of LittleFS wear leveling.

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
