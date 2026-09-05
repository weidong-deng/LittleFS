/*!
 * @file        lfs_flash_int_port.h
 *
 * @brief       LittleFS Internal Flash Port Header
 *
 * @version     V1.0.0
 *
 * @date        2026-08-07
 *
 * @attention
 *
 *  LittleFS port layer for APM32F103 internal flash
 */

#ifndef __LFS_FLASH_INT_PORT_H
#define __LFS_FLASH_INT_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include "lfs.h"

/** @addtogroup LFS_Internal_Flash_Port
  @{
*/

/** @defgroup LFS_Internal_Config
  * @{
  */

/* Internal Flash Configuration for LittleFS */
#define LFS_INT_FLASH_BASE_ADDR        0x08010000  /* Start address of LittleFS area */
#define LFS_INT_FLASH_SIZE             0x10000     /* 64KB for LittleFS */

/* Flash page size varies by chip:
 * - APM32F103: 1KB (1024 bytes) per page
 * - APM32F105/F107: 2KB (2048 bytes) per page
 */
#if defined(APM32F105xB) || defined(APM32F105xC) || defined(APM32F107xB) || defined(APM32F107xC)
#define LFS_INT_BLOCK_SIZE             2048        /* Page size of APM32F105/F107 */
#define LFS_INT_BLOCK_COUNT            32          /* 64KB / 2KB = 32 blocks */
#else
#define LFS_INT_BLOCK_SIZE             1024        /* Page size of APM32F103 */
#define LFS_INT_BLOCK_COUNT            64          /* 64KB / 1KB = 64 blocks */
#endif

#define LFS_INT_READ_SIZE              256         /* Read granularity */
#define LFS_INT_PROG_SIZE              256         /* Program granularity */
#define LFS_INT_CACHE_SIZE             256         /* Cache size */
#define LFS_INT_LOOKAHEAD_SIZE         16          /* Lookahead buffer size */

/**@} end of group LFS_Internal_Config */

/** @defgroup LFS_Internal_Exported_Functions
  * @{
  */

/* Get internal flash configuration */
const struct lfs_config *LFS_Int_GetConfig(void);

/* Block device operations */
int lfs_int_read(const struct lfs_config *c, lfs_block_t block,
                 lfs_off_t off, void *buffer, lfs_size_t size);
int lfs_int_prog(const struct lfs_config *c, lfs_block_t block,
                 lfs_off_t off, const void *buffer, lfs_size_t size);
int lfs_int_erase(const struct lfs_config *c, lfs_block_t block);
int lfs_int_sync(const struct lfs_config *c);

/* Wear leveling statistics */
void LFS_Int_GetEraseStats(uint32_t *erase_counts, uint32_t max_blocks);
void LFS_Int_PrintEraseStats(void);
void LFS_Int_ResetEraseStats(void);

/**@} end of group LFS_Internal_Exported_Functions */

/**@} end of group LFS_Internal_Flash_Port */

#ifdef __cplusplus
}
#endif

#endif /* __LFS_FLASH_INT_PORT_H */
