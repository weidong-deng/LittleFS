/*!
 * @file        lfs_spi_flash_port.c
 *
 * @brief       LittleFS SPI Flash Generic Port Implementation
 *
 * @version     V1.0.0
 *
 * @date        2026-08-07
 *
 * @attention
 *
 *  Generic SPI Flash port layer for LittleFS.
 *  This file never changes when switching flash chips -
 *  only the registered driver implementation changes.
 */

/* Includes */
#include "lfs_spi_flash_port.h"
#include <string.h>

/** @addtogroup LFS_SPI_Flash_Port
  @{
*/

/** @addtogroup LFS_SPI_Flash_Variables
  @{
  */

/* Pointer to the registered flash driver */
static const lfs_spi_flash_drv_t *s_flash_drv = NULL;

/* Static buffers for LittleFS (avoid dynamic allocation) */
static uint8_t s_read_buffer[LFS_SPI_FLASH_CACHE_SIZE];
static uint8_t s_prog_buffer[LFS_SPI_FLASH_CACHE_SIZE];
static uint8_t s_lookahead_buffer[LFS_SPI_FLASH_LOOKAHEAD_SIZE];

/* LittleFS configuration - block_count is set after driver registration */
static struct lfs_config s_lfs_config = {
    .context        = NULL,
    .read           = lfs_spi_flash_read,
    .prog           = lfs_spi_flash_prog,
    .erase          = lfs_spi_flash_erase,
    .sync           = lfs_spi_flash_sync,
    .read_size      = LFS_SPI_FLASH_READ_SIZE,
    .prog_size      = LFS_SPI_FLASH_PROG_SIZE,
    .block_size     = LFS_SPI_FLASH_BLOCK_SIZE,
    .block_count    = 0,  /* Set dynamically after driver registration */
    .block_cycles   = 500,              /* Wear leveling: move metadata after 500 erases */
    .cache_size     = LFS_SPI_FLASH_CACHE_SIZE,
    .lookahead_size = LFS_SPI_FLASH_LOOKAHEAD_SIZE,
    .read_buffer    = s_read_buffer,
    .prog_buffer    = s_prog_buffer,
    .lookahead_buffer = s_lookahead_buffer,
    .name_max       = 0,
    .file_max       = 0,
    .attr_max       = 0,
    .metadata_max   = 0,
    .inline_max     = 0,
};

/**@} end of group LFS_SPI_Flash_Variables */

/** @addtogroup LFS_SPI_Flash_Functions
  @{
  */

/*!
 * @brief       Register an external flash driver
 *
 * @param       drv     Pointer to driver interface implementation
 *
 * @retval      0 on success, negative on failure
 */
int LFS_SPIFlash_Register(const lfs_spi_flash_drv_t *drv)
{
    if (drv == NULL)
    {
        return -1;
    }

    /* Validate required function pointers */
    if (drv->read == NULL || drv->prog == NULL ||
        drv->erase == NULL || drv->sync == NULL)
    {
        return -1;
    }

    s_flash_drv = drv;

    /* Calculate block count based on flash total size and LittleFS block size */
    s_lfs_config.block_count = drv->total_size / LFS_SPI_FLASH_BLOCK_SIZE;

    return 0;
}

/*!
 * @brief       Get LittleFS configuration for the registered external flash
 *
 * @retval      Pointer to lfs_config, or NULL if no driver registered
 */
const struct lfs_config *LFS_SPIFlash_GetConfig(void)
{
    if (s_flash_drv == NULL)
    {
        return NULL;
    }
    return &s_lfs_config;
}

/*!
 * @brief       Read data from external flash via registered driver
 */
int lfs_spi_flash_read(const struct lfs_config *c, lfs_block_t block,
                       lfs_off_t off, void *buffer, lfs_size_t size)
{
    uint32_t addr;

    if (s_flash_drv == NULL)
    {
        return LFS_ERR_IO;
    }

    addr = (block * c->block_size) + off;
    return s_flash_drv->read(addr, (uint8_t *)buffer, size);
}

/*!
 * @brief       Program data to external flash via registered driver
 */
int lfs_spi_flash_prog(const struct lfs_config *c, lfs_block_t block,
                       lfs_off_t off, const void *buffer, lfs_size_t size)
{
    uint32_t addr;

    if (s_flash_drv == NULL)
    {
        return LFS_ERR_IO;
    }

    addr = (block * c->block_size) + off;
    return s_flash_drv->prog(addr, (const uint8_t *)buffer, size);
}

/*!
 * @brief       Erase a block in external flash via registered driver
 */
int lfs_spi_flash_erase(const struct lfs_config *c, lfs_block_t block)
{
    uint32_t addr;

    if (s_flash_drv == NULL)
    {
        return LFS_ERR_IO;
    }

    if (block >= c->block_count)
    {
        return LFS_ERR_INVAL;
    }

    addr = block * c->block_size;
    return s_flash_drv->erase(addr);
}

/*!
 * @brief       Sync external flash via registered driver
 */
int lfs_spi_flash_sync(const struct lfs_config *c)
{
    if (s_flash_drv == NULL)
    {
        return LFS_ERR_IO;
    }

    return s_flash_drv->sync();
}

/**@} end of group LFS_SPI_Flash_Functions */
/**@} end of group LFS_SPI_Flash_Port */
