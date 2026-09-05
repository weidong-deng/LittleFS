/*!
 * @file        lfs_spi_flash_port.h
 *
 * @brief       LittleFS SPI Flash Generic Port Header
 *
 * @version     V1.0.0
 *
 * @date        2026-08-07
 *
 * @attention
 *
 *  Generic SPI Flash port layer for LittleFS.
 *  Any SPI Flash chip can be used as long as it implements
 *  the lfs_spi_flash_drv_t interface.
 */

#ifndef __LFS_SPI_FLASH_PORT_H
#define __LFS_SPI_FLASH_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include "lfs.h"
#include <stdint.h>

/** @addtogroup LFS_SPI_Flash_Port
  @{
*/

/** @defgroup LFS_SPI_Flash_Config
  * @{
  */

/* LittleFS parameters for external SPI Flash */
#define LFS_SPI_FLASH_BLOCK_SIZE       4096        /* Erase sector size */
#define LFS_SPI_FLASH_READ_SIZE        256         /* Read granularity */
#define LFS_SPI_FLASH_PROG_SIZE        256         /* Program granularity */
#define LFS_SPI_FLASH_CACHE_SIZE       256         /* Cache size */
#define LFS_SPI_FLASH_LOOKAHEAD_SIZE   32          /* Lookahead buffer size */

/**@} end of group LFS_SPI_Flash_Config */

/** @defgroup LFS_SPI_Flash_Types
  * @{
  */

/**
 * @brief   SPI Flash driver interface
 *
 * Any SPI Flash chip (W25Qxx, GD25Qxx, MX25Lxx, etc.) only needs
 * to implement these function pointers and register via
 * LFS_SPIFlash_Register(). The port layer and LittleFS core
 * will work without any modification.
 */
typedef struct lfs_spi_flash_drv {
    /**
     * @brief   Initialize the flash chip hardware (SPI, GPIO, etc.)
     * @retval  0 on success, negative on failure
     */
    int (*init)(void);

    /**
     * @brief   Read data from flash
     * @param   addr    Absolute address in flash
     * @param   buf     Buffer to store read data
     * @param   len     Number of bytes to read
     * @retval  0 on success, negative on failure
     */
    int (*read)(uint32_t addr, uint8_t *buf, uint32_t len);

    /**
     * @brief   Program data to flash (page-level, caller ensures
     *          addr+len does not cross page boundary)
     * @param   addr    Absolute address in flash
     * @param   buf     Data buffer to write
     * @param   len     Number of bytes to write
     * @retval  0 on success, negative on failure
     */
    int (*prog)(uint32_t addr, const uint8_t *buf, uint32_t len);

    /**
     * @brief   Erase a sector (4KB typically)
     * @param   addr    Sector-aligned address
     * @retval  0 on success, negative on failure
     */
    int (*erase)(uint32_t addr);

    /**
     * @brief   Wait until flash is not busy
     * @retval  0 on success, negative on failure
     */
    int (*sync)(void);

    /* Flash geometry info (provided by the driver implementation) */
    uint32_t total_size;     /* Total flash size in bytes */
    uint32_t page_size;      /* Page size for programming */
    uint32_t sector_size;    /* Sector size for erasing */
} lfs_spi_flash_drv_t;

/**@} end of group LFS_SPI_Flash_Types */

/** @defgroup LFS_SPI_Flash_Exported_Functions
  * @{
  */

/**
 * @brief   Register an external flash driver
 *
 * Call this BEFORE LFS_SPIFlash_GetConfig(). The registered driver
 * will be used by the LittleFS port layer.
 *
 * @param   drv     Pointer to driver interface implementation
 * @retval  0 on success, negative on failure
 */
int LFS_SPIFlash_Register(const lfs_spi_flash_drv_t *drv);

/**
 * @brief   Get LittleFS configuration for the registered external flash
 * @retval  Pointer to lfs_config, or NULL if no driver registered
 */
const struct lfs_config *LFS_SPIFlash_GetConfig(void);

/* LittleFS block device callbacks (used internally, exposed for testing) */
int lfs_spi_flash_read(const struct lfs_config *c, lfs_block_t block,
                       lfs_off_t off, void *buffer, lfs_size_t size);
int lfs_spi_flash_prog(const struct lfs_config *c, lfs_block_t block,
                       lfs_off_t off, const void *buffer, lfs_size_t size);
int lfs_spi_flash_erase(const struct lfs_config *c, lfs_block_t block);
int lfs_spi_flash_sync(const struct lfs_config *c);

/**@} end of group LFS_SPI_Flash_Exported_Functions */

/**@} end of group LFS_SPI_Flash_Port */

#ifdef __cplusplus
}
#endif

#endif /* __LFS_SPI_FLASH_PORT_H */
