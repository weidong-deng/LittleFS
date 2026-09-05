/*!
 * @file        lfs_flash_int_port.c
 *
 * @brief       LittleFS Internal Flash Port Implementation
 *
 * @version     V1.0.0
 *
 * @date        2026-08-07
 *
 * @attention
 *
 *  LittleFS port layer for APM32F103 internal flash
 */

/* Includes */
#include "lfs_flash_int_port.h"
#include "apm32f10x_fmc.h"
#include <string.h>

/** @addtogroup LFS_Internal_Flash_Port
  @{
*/

/** @addtogroup LFS_Internal_Variables
  @{
  */

/* Static buffers for LittleFS (avoid dynamic allocation) */
static uint8_t lfs_int_read_buffer[LFS_INT_CACHE_SIZE];
static uint8_t lfs_int_prog_buffer[LFS_INT_CACHE_SIZE];
static uint8_t lfs_int_lookahead_buffer[LFS_INT_LOOKAHEAD_SIZE];

/* Wear leveling statistics - erase count per block */
static uint32_t lfs_int_erase_counts[LFS_INT_BLOCK_COUNT];

/* LittleFS configuration structure */
static const struct lfs_config lfs_int_config = {
    .context = NULL,
    .read = lfs_int_read,
    .prog = lfs_int_prog,
    .erase = lfs_int_erase,
    .sync = lfs_int_sync,
    .read_size = LFS_INT_READ_SIZE,
    .prog_size = LFS_INT_PROG_SIZE,
    .block_size = LFS_INT_BLOCK_SIZE,
    .block_count = LFS_INT_BLOCK_COUNT,
    .block_cycles = 500,              /* Wear leveling: move metadata after 500 erases */
    .cache_size = LFS_INT_CACHE_SIZE,
    .lookahead_size = LFS_INT_LOOKAHEAD_SIZE,
    .read_buffer = lfs_int_read_buffer,
    .prog_buffer = lfs_int_prog_buffer,
    .lookahead_buffer = lfs_int_lookahead_buffer,
    .name_max = 0,
    .file_max = 0,
    .attr_max = 0,
    .metadata_max = 0,
    .inline_max = 0,
};

/**@} end of group LFS_Internal_Variables */

/** @addtogroup LFS_Internal_Functions
  @{
  */

/*!
 * @brief       Get internal flash configuration
 *
 * @param       None
 *
 * @retval      Pointer to lfs_config structure
 */
const struct lfs_config *LFS_Int_GetConfig(void)
{
    return &lfs_int_config;
}

/*!
 * @brief       Read data from internal flash
 *
 * @param       c       LittleFS configuration
 * @param       block   Block number
 * @param       off     Offset within block
 * @param       buffer  Buffer to store read data
 * @param       size    Number of bytes to read
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int lfs_int_read(const struct lfs_config *c, lfs_block_t block,
                 lfs_off_t off, void *buffer, lfs_size_t size)
{
    uint32_t addr;
    uint32_t i;
    uint8_t *buf = (uint8_t *)buffer;

    /* Calculate actual flash address */
    addr = LFS_INT_FLASH_BASE_ADDR + (block * c->block_size) + off;

    /* Internal flash is memory-mapped, can read directly */
    for (i = 0; i < size; i++)
    {
        buf[i] = *(volatile uint8_t *)addr;
        addr++;
    }

    return LFS_ERR_OK;
}

/*!
 * @brief       Program data to internal flash
 *
 * @param       c       LittleFS configuration
 * @param       block   Block number
 * @param       off     Offset within block
 * @param       buffer  Data buffer to write
 * @param       size    Number of bytes to write
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int lfs_int_prog(const struct lfs_config *c, lfs_block_t block,
                 lfs_off_t off, const void *buffer, lfs_size_t size)
{
    uint32_t addr;
    uint32_t i;
    const uint8_t *buf = (const uint8_t *)buffer;
    FMC_STATUS_T status;

    /* Calculate actual flash address */
    addr = LFS_INT_FLASH_BASE_ADDR + (block * c->block_size) + off;

    /* Unlock flash */
    FMC_Unlock();

    /* Program data (half-word at a time for better performance) */
    for (i = 0; i < size; i += 2)
    {
        uint16_t data;
        
        if (i + 1 < size)
        {
            data = buf[i] | (buf[i + 1] << 8);
        }
        else
        {
            data = buf[i];
        }

        status = FMC_ProgramHalfWord(addr, data);
        if (status != FMC_STATUS_COMPLETE)
        {
            FMC_Lock();
            return LFS_ERR_IO;
        }
        addr += 2;
    }

    /* Lock flash */
    FMC_Lock();

    return LFS_ERR_OK;
}

/*!
 * @brief       Erase a block in internal flash
 *
 * @param       c       LittleFS configuration
 * @param       block   Block number
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int lfs_int_erase(const struct lfs_config *c, lfs_block_t block)
{
    uint32_t addr;
    FMC_STATUS_T status;

    /* Check block bounds */
    if (block >= c->block_count)
    {
        return LFS_ERR_INVAL;
    }

    /* Calculate actual flash address */
    addr = LFS_INT_FLASH_BASE_ADDR + (block * c->block_size);

    /* Unlock flash */
    FMC_Unlock();

    /* Erase page */
    status = FMC_ErasePage(addr);

    /* Lock flash */
    FMC_Lock();

    if (status != FMC_STATUS_COMPLETE)
    {
        return LFS_ERR_IO;
    }

    /* Increment erase count for this block */
    if (block < LFS_INT_BLOCK_COUNT)
    {
        lfs_int_erase_counts[block]++;
    }

    return LFS_ERR_OK;
}

/*!
 * @brief       Sync internal flash (no-op for internal flash)
 *
 * @param       c       LittleFS configuration
 *
 * @retval      LFS_ERR_OK
 */
int lfs_int_sync(const struct lfs_config *c)
{
    /* Internal flash operations are synchronous, nothing to do */
    return LFS_ERR_OK;
}

/*!
 * @brief       Get erase statistics
 *
 * @param       erase_counts    Array to store erase counts
 * @param       max_blocks      Maximum number of blocks to report
 */
void LFS_Int_GetEraseStats(uint32_t *erase_counts, uint32_t max_blocks)
{
    uint32_t i;
    uint32_t count = (max_blocks < LFS_INT_BLOCK_COUNT) ? max_blocks : LFS_INT_BLOCK_COUNT;

    for (i = 0; i < count; i++)
    {
        erase_counts[i] = lfs_int_erase_counts[i];
    }
}

/*!
 * @brief       Print erase statistics to stdout
 */
void LFS_Int_PrintEraseStats(void)
{
    uint32_t i;
    uint32_t min_erase = 0xFFFFFFFF, max_erase = 0;
    uint32_t total_erases = 0;
    uint32_t blocks_with_erases = 0;
    
    /* Separate metadata blocks (first few blocks) from data blocks */
    uint32_t metadata_blocks = 4;  /* Blocks 0-3 typically used for metadata */
    uint32_t data_min_erase = 0xFFFFFFFF, data_max_erase = 0;
    uint32_t data_total_erases = 0;
    uint32_t data_blocks_with_erases = 0;

    printf("\n=== Block Erase Statistics ===\n");

    for (i = 0; i < LFS_INT_BLOCK_COUNT; i++)
    {
        if (lfs_int_erase_counts[i] > 0)
        {
            blocks_with_erases++;
            total_erases += lfs_int_erase_counts[i];
            if (lfs_int_erase_counts[i] < min_erase)
                min_erase = lfs_int_erase_counts[i];
            if (lfs_int_erase_counts[i] > max_erase)
                max_erase = lfs_int_erase_counts[i];
            
            /* Track data blocks separately (skip metadata blocks) */
            if (i >= metadata_blocks)
            {
                data_blocks_with_erases++;
                data_total_erases += lfs_int_erase_counts[i];
                if (lfs_int_erase_counts[i] < data_min_erase)
                    data_min_erase = lfs_int_erase_counts[i];
                if (lfs_int_erase_counts[i] > data_max_erase)
                    data_max_erase = lfs_int_erase_counts[i];
            }
        }
    }

    /* Print blocks with erase counts (compact format) */
    printf("Block erase counts (block:count):\n");
    for (i = 0; i < LFS_INT_BLOCK_COUNT; i++)
    {
        if (lfs_int_erase_counts[i] > 0)
        {
            printf("  [%2ld]:%3ld", i, lfs_int_erase_counts[i]);
            if ((blocks_with_erases > 0) && ((i + 1) % 8 == 0))
                printf("\n");
        }
    }
    printf("\n");

    /* Print summary */
    printf("Summary:\n");
    printf("  Total blocks: %d\n", LFS_INT_BLOCK_COUNT);
    printf("  Blocks with erases: %ld\n", blocks_with_erases);
    printf("  Total erases: %ld\n", total_erases);
    if (blocks_with_erases > 0)
    {
        printf("  Min erases: %ld\n", min_erase);
        printf("  Max erases: %ld\n", max_erase);
        printf("  Overall wear distribution ratio: %.2f\n",
               (float)max_erase / (min_erase > 0 ? min_erase : 1));
    }
    
    /* Print data blocks summary (excluding metadata blocks) */
    printf("\nData Blocks Summary (blocks %ld-%d):\n", metadata_blocks, LFS_INT_BLOCK_COUNT - 1);
    printf("  Data blocks with erases: %ld\n", data_blocks_with_erases);
    printf("  Data block total erases: %ld\n", data_total_erases);
    if (data_blocks_with_erases > 0)
    {
        printf("  Data block min erases: %ld\n", data_min_erase);
        printf("  Data block max erases: %ld\n", data_max_erase);
        printf("  Data block wear distribution ratio: %.2f\n",
               (float)data_max_erase / (data_min_erase > 0 ? data_min_erase : 1));
    }
}

/*!
 * @brief       Reset erase statistics
 */
void LFS_Int_ResetEraseStats(void)
{
    memset(lfs_int_erase_counts, 0, sizeof(lfs_int_erase_counts));
}

/**@} end of group LFS_Internal_Functions */
/**@} end of group LFS_Internal_Flash_Port */
