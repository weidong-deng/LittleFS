/*!
 * @file        w25qxx_flash_drv.h
 *
 * @brief       W25QXX Flash Driver - implements lfs_spi_flash_drv_t interface
 *
 * @version     V1.0.0
 *
 * @date        2026-08-07
 *
 * @attention
 *
 *  This is a concrete flash driver implementation.
 *  To use a different flash chip, create a new driver file
 *  that implements the same lfs_spi_flash_drv_t interface.
 */

#ifndef __W25QXX_FLASH_DRV_H
#define __W25QXX_FLASH_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include "lfs_spi_flash_port.h"

/** @addtogroup W25QXX_Flash_Driver
  @{
*/

/** @defgroup W25QXX_Config
  * @{
  */

/* SPI Interface Configuration */
#define W25QXX_SPI                     SPI1
#define W25QXX_SPI_CLK                 RCM_APB2_PERIPH_SPI1

#define W25QXX_CS_PORT                 GPIOA
#define W25QXX_CS_PIN                  GPIO_PIN_4
#define W25QXX_CS_CLK                  RCM_APB2_PERIPH_GPIOA

/* W25QXX Commands */
#define W25QXX_CMD_WRITE_ENABLE        0x06
#define W25QXX_CMD_WRITE_DISABLE       0x04
#define W25QXX_CMD_READ_STATUS_REG1    0x05
#define W25QXX_CMD_READ_STATUS_REG2    0x35
#define W25QXX_CMD_WRITE_STATUS_REG    0x01
#define W25QXX_CMD_PAGE_PROGRAM        0x02
#define W25QXX_CMD_READ_DATA           0x03
#define W25QXX_CMD_FAST_READ           0x0B
#define W25QXX_CMD_SECTOR_ERASE        0x20
#define W25QXX_CMD_BLOCK_ERASE_32K     0x52
#define W25QXX_CMD_BLOCK_ERASE_64K     0xD8
#define W25QXX_CMD_CHIP_ERASE          0xC7
#define W25QXX_CMD_POWER_DOWN          0xB9
#define W25QXX_CMD_RELEASE_POWER_DOWN  0xAB
#define W25QXX_CMD_JEDEC_ID            0x9F

/* W25QXX Status Register Bits */
#define W25QXX_STATUS_BUSY             (1 << 0)
#define W25QXX_STATUS_WEL              (1 << 1)

/* W25QXX Device IDs */
#define W25QXX_ID_W25Q16               0x4015
#define W25QXX_ID_W25Q32               0x4016
#define W25QXX_ID_W25Q64               0x4017
#define W25QXX_ID_W25Q128              0x4018

/* W25QXX Geometry */
#define W25QXX_PAGE_SIZE               256
#define W25QXX_SECTOR_SIZE             4096

/* Default flash size (override if needed) */
#ifndef W25QXX_TOTAL_SIZE
#define W25QXX_TOTAL_SIZE              (1024 * 1024)  /* 1MB default (W25Q64) */
#endif

/**@} end of group W25QXX_Config */

/** @defgroup W25QXX_Exported_Functions
  * @{
  */

/**
 * @brief   Get the W25QXX driver instance
 *
 * Returns a pointer to the lfs_spi_flash_drv_t structure
 * that can be registered with LFS_SPIFlash_Register().
 *
 * @retval  Pointer to W25QXX driver interface
 */
const lfs_spi_flash_drv_t *W25QXX_GetDriver(void);

/* Low-level functions (exposed for direct use if needed) */
uint16_t W25QXX_ReadID(void);
void W25QXX_ChipErase(void);

/**@} end of group W25QXX_Exported_Functions */

/**@} end of group W25QXX_Flash_Driver */

#ifdef __cplusplus
}
#endif

#endif /* __W25QXX_FLASH_DRV_H */
