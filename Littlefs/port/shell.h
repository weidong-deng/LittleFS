/*!
 * @file        shell.h
 *
 * @brief       LittleFS Interactive Shell
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
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the GEEHY SOFTWARE PACKAGE LICENSE for the governing permissions
 *  and limitations under the License.
 */

#ifndef __SHELL_H
#define __SHELL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include "lfs_app.h"

/** @addtogroup LFS_Shell
  @{
*/

/** @defgroup Shell_Config
  * @{
  */

#define SHELL_CMD_MAX_LEN       128     /* Maximum command line length */
#define SHELL_MAX_ARGS          8       /* Maximum number of arguments */
#define SHELL_PATH_MAX_LEN      256     /* Maximum path length */
#define SHELL_PROMPT            "lfs> " /* Shell prompt string */

/**@} end of group Shell_Config */

/** @defgroup Shell_Exported_Functions
  * @{
  */

/**
 * @brief  Initialize shell, set storage type and mount filesystem
 * @param  storage: LFS_STORAGE_INTERNAL or LFS_STORAGE_EXTERNAL
 * @retval 0 on success, negative on failure
 */
int Shell_Init(LFS_Storage_t storage);

/**
 * @brief  Process a single command line string
 * @param  line: command line input (null-terminated)
 */
void Shell_ProcessLine(const char *line);

/**
 * @brief  Feed one character to shell (call from USART RX handler)
 * @param  ch: received character
 * @note   When '\r' or '\n' is received, the buffered line is processed
 */
void Shell_PutChar(char ch);

/**
 * @brief  Print the shell prompt
 */
void Shell_PrintPrompt(void);

/**
 * @brief  Shell task - call in main loop to process input
 * @note   Handles character echo, buffering, and command execution.
 *         All printf calls happen here, NOT in interrupt context.
 */
void Shell_Task(void);

/**@} end of group Shell_Exported_Functions */

/**@} end of group LFS_Shell */

#ifdef __cplusplus
}
#endif

#endif /* __SHELL_H */
