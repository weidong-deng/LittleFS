/*!
 * @file        w25qxx_flash_drv.c
 *
 * @brief       W25QXX Flash Driver Implementation
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

/* Includes */
#include "w25qxx_flash_drv.h"
#include "apm32f10x_spi.h"
#include "apm32f10x_gpio.h"
#include "apm32f10x_rcm.h"

/** @addtogroup W25QXX_Flash_Driver
  @{
*/

/** @addtogroup W25QXX_LowLevel
  @{
  */

/*!
 * @brief       Set CS pin low
 */
static void W25QXX_CS_Low(void)
{
    W25QXX_CS_PORT->BC = W25QXX_CS_PIN;
}

/*!
 * @brief       Set CS pin high
 */
static void W25QXX_CS_High(void)
{
    W25QXX_CS_PORT->BSC = W25QXX_CS_PIN;
}

/*!
 * @brief       Read and write data over SPI
 */
static uint8_t W25QXX_SPI_ReadWrite(uint8_t data)
{
    while (SPI_I2S_ReadStatusFlag(W25QXX_SPI, SPI_FLAG_TXBE) == RESET);
    SPI_I2S_TxData(W25QXX_SPI, data);
    while (SPI_I2S_ReadStatusFlag(W25QXX_SPI, SPI_FLAG_RXBNE) == RESET);
    return SPI_I2S_RxData(W25QXX_SPI);
}

/*!
 * @brief       Write enable the flash
 */
static void W25QXX_WriteEnable(void)
{
    W25QXX_CS_Low();
    W25QXX_SPI_ReadWrite(W25QXX_CMD_WRITE_ENABLE);
    W25QXX_CS_High();
}

/*!
 * @brief       Read status register from the flash
 */
static uint8_t W25QXX_ReadStatus(void)
{
    uint8_t status;
    W25QXX_CS_Low();
    W25QXX_SPI_ReadWrite(W25QXX_CMD_READ_STATUS_REG1);
    status = W25QXX_SPI_ReadWrite(0xFF);
    W25QXX_CS_High();
    return status;
}

/**@} end of group W25QXX_LowLevel */

/** @addtogroup W25QXX_Interface
  * @{
  */

/*!
 * @brief       Initialize W25QXX SPI hardware
 *
 * @retval      0 on success, -1 on failure
 */
static int w25qxx_init(void)
{
    GPIO_Config_T GPIO_configStruct;
    SPI_Config_T SPI_configStruct;

    /* Enable GPIO and SPI clocks */
    RCM_EnableAPB2PeriphClock(W25QXX_CS_CLK | W25QXX_SPI_CLK |
                               RCM_APB2_PERIPH_GPIOA | RCM_APB2_PERIPH_AFIO);

    /* Configure CS pin as output push-pull */
    GPIO_configStruct.pin = W25QXX_CS_PIN;
    GPIO_configStruct.mode = GPIO_MODE_OUT_PP;
    GPIO_configStruct.speed = GPIO_SPEED_50MHz;
    GPIO_Config(W25QXX_CS_PORT, &GPIO_configStruct);
    W25QXX_CS_High();

    /* Configure SPI pins: PA5-SCK, PA6-MISO, PA7-MOSI */
    GPIO_configStruct.pin = GPIO_PIN_5 | GPIO_PIN_7;
    GPIO_configStruct.mode = GPIO_MODE_AF_PP;
    GPIO_configStruct.speed = GPIO_SPEED_50MHz;
    GPIO_Config(GPIOA, &GPIO_configStruct);

    GPIO_configStruct.pin = GPIO_PIN_6;
    GPIO_configStruct.mode = GPIO_MODE_IN_FLOATING;
    GPIO_Config(GPIOA, &GPIO_configStruct);

    /* Configure SPI */
    SPI_ConfigStructInit(&SPI_configStruct);
    SPI_configStruct.mode = SPI_MODE_MASTER;
    SPI_configStruct.direction = SPI_DIRECTION_2LINES_FULLDUPLEX;
    SPI_configStruct.length = SPI_DATA_LENGTH_8B;
    SPI_configStruct.polarity = SPI_CLKPOL_LOW;
    SPI_configStruct.phase = SPI_CLKPHA_1EDGE;
    SPI_configStruct.nss = SPI_NSS_SOFT;
    SPI_configStruct.baudrateDiv = SPI_BAUDRATE_DIV_8;  /* 72MHz/8 = 9MHz */
    SPI_configStruct.firstBit = SPI_FIRSTBIT_MSB;
    SPI_configStruct.crcPolynomial = 7;
    SPI_Config(W25QXX_SPI, &SPI_configStruct);

    /* Enable SPI */
    SPI_Enable(W25QXX_SPI);

    /* Release power down mode */
    W25QXX_CS_Low();
    W25QXX_SPI_ReadWrite(W25QXX_CMD_RELEASE_POWER_DOWN);
    W25QXX_CS_High();

    /* Wait for device ready */
    while (W25QXX_ReadStatus() & W25QXX_STATUS_BUSY);

    /* Verify device presence */
    uint16_t id = W25QXX_ReadID();
    if (id == 0x0000 || id == 0xFFFF)
    {
        return -1;
    }

    return 0;
}

/*!
 * @brief       Read data from W25QXX
 *
 * @param[in]   addr    Absolute address in flash
 * @param[out]  buf     Buffer to store read data
 * @param[in]   len     Number of bytes to read
 *
 * @retval      0 on success, -1 on failure
 */
static int w25qxx_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint32_t i;

    /* Wait until not busy */
    while (W25QXX_ReadStatus() & W25QXX_STATUS_BUSY);

    W25QXX_CS_Low();
    W25QXX_SPI_ReadWrite(W25QXX_CMD_READ_DATA);
    W25QXX_SPI_ReadWrite((addr >> 16) & 0xFF);
    W25QXX_SPI_ReadWrite((addr >> 8) & 0xFF);
    W25QXX_SPI_ReadWrite(addr & 0xFF);

    for (i = 0; i < len; i++)
    {
        buf[i] = W25QXX_SPI_ReadWrite(0xFF);
    }
    W25QXX_CS_High();

    return 0;
}

/*!
 * @brief       Program data to W25QXX (handles page boundary)
 *
 * @param[in]   addr    Absolute address in flash
 * @param[in]   buf     Data buffer to write
 * @param[in]   len     Number of bytes to write
 *
 * @retval      0 on success, -1 on failure
 */
static int w25qxx_prog(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    uint32_t page_remain;
    uint32_t i;

    page_remain = W25QXX_PAGE_SIZE - (addr % W25QXX_PAGE_SIZE);
    if (len < page_remain)
    {
        page_remain = len;
    }

    while (1)
    {
        /* Wait until not busy */
        while (W25QXX_ReadStatus() & W25QXX_STATUS_BUSY);

        W25QXX_WriteEnable();

        W25QXX_CS_Low();
        W25QXX_SPI_ReadWrite(W25QXX_CMD_PAGE_PROGRAM);
        W25QXX_SPI_ReadWrite((addr >> 16) & 0xFF);
        W25QXX_SPI_ReadWrite((addr >> 8) & 0xFF);
        W25QXX_SPI_ReadWrite(addr & 0xFF);

        for (i = 0; i < page_remain; i++)
        {
            W25QXX_SPI_ReadWrite(buf[i]);
        }
        W25QXX_CS_High();

        if (len == page_remain)
        {
            break;
        }

        buf += page_remain;
        addr += page_remain;
        len -= page_remain;

        page_remain = (len > W25QXX_PAGE_SIZE) ? W25QXX_PAGE_SIZE : len;
    }

    return 0;
}

/*!
 * @brief       Erase a sector (4KB)
 *
 * @param[in]   addr    Sector-aligned address
 *
 * @retval      0 on success, -1 on failure
 */
static int w25qxx_erase(uint32_t addr)
{
    /* Wait until not busy */
    while (W25QXX_ReadStatus() & W25QXX_STATUS_BUSY);

    W25QXX_WriteEnable();

    W25QXX_CS_Low();
    W25QXX_SPI_ReadWrite(W25QXX_CMD_SECTOR_ERASE);
    W25QXX_SPI_ReadWrite((addr >> 16) & 0xFF);
    W25QXX_SPI_ReadWrite((addr >> 8) & 0xFF);
    W25QXX_SPI_ReadWrite(addr & 0xFF);
    W25QXX_CS_High();

    /* Wait for erase to complete */
    while (W25QXX_ReadStatus() & W25QXX_STATUS_BUSY);

    return 0;
}

/*!
 * @brief       Wait until W25QXX is not busy
 *
 * @retval      0 on success
 */
static int w25qxx_sync(void)
{
    while (W25QXX_ReadStatus() & W25QXX_STATUS_BUSY);
    return 0;
}

/**@} end of group W25QXX_Interface */

/** @addtogroup W25QXX_Driver_Instance
  @{
  */

/* W25QXX driver instance - implements lfs_spi_flash_drv_t */
static const lfs_spi_flash_drv_t s_w25qxx_drv = {
    .init        = w25qxx_init,
    .read        = w25qxx_read,
    .prog        = w25qxx_prog,
    .erase       = w25qxx_erase,
    .sync        = w25qxx_sync,
    .total_size  = W25QXX_TOTAL_SIZE,
    .page_size   = W25QXX_PAGE_SIZE,
    .sector_size = W25QXX_SECTOR_SIZE,
};

/*!
 * @brief       Get the W25QXX driver instance
 *
 * @retval      Pointer to W25QXX driver interface
 */
const lfs_spi_flash_drv_t *W25QXX_GetDriver(void)
{
    return &s_w25qxx_drv;
}

/*!
 * @brief       Read W25QXX JEDEC ID
 *
 * @retval      Device ID (Manufacturer ID + Device ID)
 */
uint16_t W25QXX_ReadID(void)
{
    uint16_t id;

    W25QXX_CS_Low();
    W25QXX_SPI_ReadWrite(W25QXX_CMD_JEDEC_ID);
    id = (uint16_t)W25QXX_SPI_ReadWrite(0xFF) << 8;
    id |= W25QXX_SPI_ReadWrite(0xFF);
    W25QXX_CS_High();

    return id;
}

/*!
 * @brief       Erase entire chip
 */
void W25QXX_ChipErase(void)
{
    while (W25QXX_ReadStatus() & W25QXX_STATUS_BUSY);

    W25QXX_WriteEnable();

    W25QXX_CS_Low();
    W25QXX_SPI_ReadWrite(W25QXX_CMD_CHIP_ERASE);
    W25QXX_CS_High();

    while (W25QXX_ReadStatus() & W25QXX_STATUS_BUSY);
}

/**@} end of group W25QXX_Driver_Instance */
/**@} end of group W25QXX_Flash_Driver */
