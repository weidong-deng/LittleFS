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

  This demo demonstrates the power loss resilience capability of LittleFS
  on APM32F107 internal flash. It uses KEY1 button to simulate sudden
  power loss during write operations, verifying that data remains
  consistent and the file system can recover gracefully.

  The demo runs in two phases:

  Phase 1 - Recovery Verification:
  - Mounts the file system (proves FS is not corrupted)
  - Reads the counter file and data file
  - Verifies CRC32 and byte pattern integrity of the data block
  - Checks counter consistency with data sequence number
  - Reports PASS/FAIL for each verification step

  Phase 2 - Continuous Write:
  - Reads existing counter from flash (if any)
  - Continuously writes incrementing counter + data blocks in a loop
  - Each data block (512 bytes) contains: sequence number, CRC32 of
    payload, and a fill pattern (all bytes = seq & 0xFF)
  - KEY1: Simulates sudden power loss (immediate NVIC_SystemReset,
    no unmount/close -- like real power failure)
  - KEY2: Stops the write loop gracefully

  USART1 is used for output at 115200 baud, 8N1.

  Test procedure:
  1. Power on / reset the board
  2. Phase 1 verifies any previously written data (first boot: no data)
  3. Phase 2 starts continuous writing
  4. Press KEY1 at any time to simulate power loss
  5. Board resets, go back to step 2
  6. Observe that Phase 1 always passes, proving data integrity

&par Directory contents

  - Source/main.c                  Main program
  - Source/system_apm32f10x.c      System clock configuration
  - Source/apm32f10x_int.c         Interrupt handlers
  - Include/main.h                 Main header
  - Include/apm32f10x_int.h        Interrupt handlers header

&par Hardware environment

  - APM32F107 development board
  - USART1 TX: PA9  (115200 baud, 8N1)
  - KEY1: PA1 (active low, press to simulate power loss)
  - KEY2: PA0 (active low, press to stop continuous write)
  - Internal flash area: 0x08010000 - 0x0801FFFF (64KB)

&par Notes

  The demo maintains two files:
  - /power_loss_counter.dat: 4-byte counter value
  - /power_loss_data.dat: 512-byte data block with CRC32

  When KEY1 is pressed during Phase 2, the system resets immediately
  without unmounting the file system, simulating a real power failure.
  On the next boot, Phase 1 verifies that the data is still intact.

*/
