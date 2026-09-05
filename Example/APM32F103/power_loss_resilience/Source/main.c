/*!
 * @file        main.c
 *
 * @brief       Main program body
 *
 * @version     V1.0.0
 *
 * @date        2026-08-09
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

/* Includes */
#include "main.h"
#include "lfs_app.h"
#include "lfs_flash_int_port.h"
#include "lfs_spi_flash_port.h"

void USART_Init(void);
static int continuous_write_test(LFS_Storage_t storage);
static int recovery_verify_test(LFS_Storage_t storage);

#define COUNTER_FILE_PATH       "/power_loss_counter.dat"
#define DATA_FILE_PATH          "/power_loss_data.dat"
#define DATA_SIZE               512

/** @addtogroup Examples
  @{
  */

/** @addtogroup Template
  @{
  */

/** @defgroup Template_Functions
  @{
  */

/*!
 * @brief       Main program
 *
 * @param       None
 *
 * @retval      None
 *
 */
int main(void)
{
    /* System clocks configuration */
    SystemClockConfig();
    USART_Init();

    /* Initialize buttons */
    BOARD_PBInit(BUTTON_KEY1, BUTTON_MODE_GPIO);
    BOARD_PBInit(BUTTON_KEY2, BUTTON_MODE_GPIO);

    printf("\n=== LittleFS Power Loss Resilience Demo ===\n");
    printf("Flash Area: 0x08010000 - 0x0801FFFF (64KB)\n");
    printf("Block Size: %dKB, Block Count: %d\n\n", LFS_INT_BLOCK_SIZE/1024, LFS_INT_BLOCK_COUNT);
    printf("Press KEY1 to simulate power loss during write operations\n");
    printf("KEY2: Stop continuous write\n\n");

    LFS_App_Init(LFS_STORAGE_INTERNAL);

    /* Phase 1: Verify recovery from any previous power loss */
    recovery_verify_test(LFS_STORAGE_INTERNAL);

    /* Phase 2: Continuous write with power loss simulation */
    continuous_write_test(LFS_STORAGE_INTERNAL);

    printf("\n=== Test Complete ===\n");

    while(1)
    {
    }
}

/*
 * Data block structure: each write stores a counter + CRC32 so that
 * after a power loss we can verify the data is either the complete
 * previous block or the complete current block -- never a torn write.
 */
typedef struct {
    uint32_t seq;       /* Sequence number (incremented each write) */
    uint32_t crc32;     /* CRC32 of payload */
    uint8_t  payload[DATA_SIZE - 8]; /* Fill pattern: all bytes = (seq & 0xFF) */
} DataBlock_t;

static uint32_t calc_crc32(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFF;
    uint32_t i;
    int j;
    for (i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFF;
}

static void fill_block(DataBlock_t *block, uint32_t seq)
{
    block->seq = seq;
    memset(block->payload, (uint8_t)(seq & 0xFF), sizeof(block->payload));
    block->crc32 = calc_crc32(block->payload, sizeof(block->payload));
}

static int verify_block(const DataBlock_t *block)
{
    uint32_t expected_crc = calc_crc32(block->payload, sizeof(block->payload));
    uint8_t pattern = (uint8_t)(block->seq & 0xFF);
    uint32_t i;

    /* Check payload pattern */
    for (i = 0; i < sizeof(block->payload); i++)
    {
        if (block->payload[i] != pattern)
        {
            printf("  FAIL: Payload byte %ld mismatch: 0x%02X (expected 0x%02X)\n",
                   i, block->payload[i], pattern);
            return -1;
        }
    }

    /* Check CRC */
    if (block->crc32 != expected_crc)
    {
        printf("  FAIL: CRC32 mismatch: stored=0x%08lX, calc=0x%08lX\n",
               block->crc32, expected_crc);
        return -1;
    }

    return 0;
}

/*!
 * @brief       Recovery verification after power loss
 *
 *  Called on every boot. Mounts the filesystem, reads back the counter
 *  and data file, and verifies everything is consistent. This proves
 *  that even if power was lost mid-write, the filesystem is intact.
 */
static int recovery_verify_test(LFS_Storage_t storage)
{
    int err;
    uint32_t counter = 0;
    DataBlock_t block;

    printf("\n=== Phase 1: Recovery Verification ===\n");

    /* Step 1: Mount filesystem -- if this succeeds after a reset
     * during write, it already proves FS is not corrupted */
    err = LFS_App_Mount(storage);
    if (err != LFS_ERR_OK)
    {
        printf("  [FAIL] Mount failed: %d\n", err);
        printf("  Filesystem is CORRUPTED after power loss!\n");
        return err;
    }
    printf("  [PASS] Filesystem mounted successfully\n");

    /* Step 2: Read counter file */
    err = LFS_App_OpenFile(storage, COUNTER_FILE_PATH, LFS_MODE_READ);
    if (err != LFS_ERR_OK)
    {
        printf("  [INFO] No counter file found -- first boot\n");
        LFS_App_Unmount(storage);
        return 0;
    }

    err = LFS_App_ReadFile(storage, &counter, sizeof(counter));
    LFS_App_CloseFile(storage);

    if (err < 0)
    {
        printf("  [FAIL] Counter read failed: %d\n", err);
        LFS_App_Unmount(storage);
        return err;
    }
    printf("  [PASS] Counter recovered: %lu\n", counter);

    /* Step 3: Read and verify data file */
    err = LFS_App_OpenFile(storage, DATA_FILE_PATH, LFS_MODE_READ);
    if (err != LFS_ERR_OK)
    {
        printf("  [FAIL] Data file open failed: %d\n", err);
        LFS_App_Unmount(storage);
        return err;
    }

    err = LFS_App_ReadFile(storage, &block, sizeof(DataBlock_t));
    LFS_App_CloseFile(storage);

    if (err < 0)
    {
        printf("  [FAIL] Data file read failed: %d\n", err);
        LFS_App_Unmount(storage);
        return err;
    }

    printf("  Data block: seq=%lu, crc32=0x%08lX\n", block.seq, block.crc32);

    if (verify_block(&block) == 0)
    {
        printf("  [PASS] Data integrity verified (CRC32 + pattern)\n");
    }
    else
    {
        printf("  [FAIL] Data corruption detected!\n");
        LFS_App_Unmount(storage);
        return LFS_ERR_CORRUPT;
    }

    /* Step 4: Check counter matches data seq */
    if (block.seq == counter)
    {
        printf("  [PASS] Counter (%lu) matches data seq (%lu)\n", counter, block.seq);
    }
    else
    {
        printf("  [WARN] Counter (%lu) != data seq (%lu)\n", counter, block.seq);
        printf("         This is expected if power was lost between writes.\n");
    }

    printf("\n  >>> Recovery verification PASSED <<<\n");
    printf("  Filesystem survived power loss without corruption.\n");

    LFS_App_Unmount(storage);
    return LFS_ERR_OK;
}

/*!
 * @brief       Continuous write test with power loss simulation
 *
 *  Continuously writes incrementing data blocks. KEY1 triggers an
 *  immediate reset (simulating power loss) at any point during the
 *  write cycle. KEY2 stops the test gracefully.
 */
static int continuous_write_test(LFS_Storage_t storage)
{
    int err;
    uint32_t counter = 0;
    DataBlock_t block;
    uint32_t write_count = 0;

    printf("\n=== Phase 2: Continuous Write (press KEY1 to cut power) ===\n");

    err = LFS_App_Mount(storage);
    if (err != LFS_ERR_OK)
    {
        printf("  Mount failed: %d\n", err);
        return err;
    }

    /* Read existing counter */
    err = LFS_App_OpenFile(storage, COUNTER_FILE_PATH, LFS_MODE_READ);
    if (err == LFS_ERR_OK)
    {
        LFS_App_ReadFile(storage, &counter, sizeof(counter));
        LFS_App_CloseFile(storage);
        printf("  Resuming from counter: %lu\n", counter);
    }
    else
    {
        printf("  Starting fresh from counter: 0\n");
        counter = 0;
    }

    printf("\n  Writing continuously... Press KEY1 to simulate power loss.\n");
    printf("  (KEY2 to stop gracefully)\n\n");

    while (1)
    {
        counter++;
        fill_block(&block, counter);

        /* Write counter file */
        err = LFS_App_OpenFile(storage, COUNTER_FILE_PATH, LFS_MODE_WRITE);
        if (err != LFS_ERR_OK)
        {
            printf("  [%lu] Counter open failed: %d\n", counter, err);
            break;
        }
        err = LFS_App_WriteFile(storage, &counter, sizeof(counter));
        LFS_App_CloseFile(storage);
        if (err < 0)
        {
            printf("  [%lu] Counter write failed: %d\n", counter, err);
            break;
        }

        /* Write data file */
        err = LFS_App_OpenFile(storage, DATA_FILE_PATH, LFS_MODE_WRITE);
        if (err != LFS_ERR_OK)
        {
            printf("  [%lu] Data open failed: %d\n", counter, err);
            break;
        }
        err = LFS_App_WriteFile(storage, &block, sizeof(DataBlock_t));
        LFS_App_CloseFile(storage);
        if (err < 0)
        {
            printf("  [%lu] Data write failed: %d\n", counter, err);
            break;
        }

        write_count++;

        /* Progress every 10 writes */
        if (write_count % 10 == 0)
        {
            printf("  [%lu] seq=%lu written OK\n", write_count, counter);
        }

        /* Check KEY1: simulate sudden power loss (immediate reset) */
        if (BOARD_PBGetState(BUTTON_KEY1) == 0)
        {
            printf("\n  >>> KEY1 PRESSED - CUTTING POWER NOW <<<\n");
            printf("  Current seq: %lu (may or may not be persisted)\n", counter);
            printf("  !!! SIMULATED POWER LOSS !!!\n");
            /* No unmount, no close -- just reset, like real power loss */
            NVIC_SystemReset();
        }

        /* Check KEY2: stop gracefully */
        if (BOARD_PBGetState(BUTTON_KEY2) == 0)
        {
            printf("\n  >>> KEY2 PRESSED - Stopping gracefully <<<\n");
            break;
        }
    }

    LFS_App_Unmount(storage);

    printf("\n=== Write Session Summary ===\n");
    printf("  Total writes completed: %lu\n", write_count);
    printf("  Final counter value: %lu\n", counter);
    printf("\n  >>> Reset the board now to verify data survived <<<\n");
    printf("  >>> (or press KEY1 during next run to test again) <<<\n");

    return LFS_ERR_OK;
}

/*!
 * @brief       USART Init
 *
 * @param       None
 *
 * @retval      None
 *
 */
void USART_Init(void)
{
    /* USART Init */
    USART_Config_T usart_configStruct;
    usart_configStruct.baudRate = 115200;
    usart_configStruct.hardwareFlow = USART_HARDWARE_FLOW_NONE;
    usart_configStruct.mode = USART_MODE_TX_RX;
    usart_configStruct.parity = USART_PARITY_NONE;
    usart_configStruct.stopBits = USART_STOP_BIT_1;
    usart_configStruct.wordLength = USART_WORD_LEN_8B;

    BOARD_COMInit(COM1, &usart_configStruct);

}

#if defined (__CC_ARM) || defined (__ICCARM__) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))

/*!
* @brief       Redirect C Library function printf to serial port.
*              After Redirection, you can use printf function.
*
* @param       ch:  The characters that need to be send.
*
* @param       *f:  pointer to a FILE that can recording all information
*              needed to control a stream
*
* @retval      The characters that need to be send.
*
* @note
*/
int fputc(int ch, FILE* f)
{
    /* send a byte of data to the serial port */
    USART_TxData(USART1, (uint8_t)ch);

    /* wait for the data to be send */
    while (USART_ReadStatusFlag(USART1, USART_FLAG_TXBE) == RESET);

    return (ch);
}

#elif defined (__GNUC__)

/*!
* @brief       Redirect C Library function printf to serial port.
*              After Redirection, you can use printf function.
*
* @param       ch:  The characters that need to be send.
*
* @retval      The characters that need to be send.
*
* @note
*/
int __io_putchar(int ch)
{
    /* send a byte of data to the serial port */
    USART_TxData(USART1, ch);

    /* wait for the data to be send */
    while (USART_ReadStatusFlag(USART1, USART_FLAG_TXBE) == RESET);

    return ch;
}

/*!
* @brief       Redirect C Library function printf to serial port.
*              After Redirection, you can use printf function.
*
* @param       file:  Meaningless in this function.
*
* @param       *ptr:  Buffer pointer for data to be sent.
*
* @param       len:  Length of data to be sent.
*
* @retval      The characters that need to be send.
*
* @note
*/
int _write(int file, char* ptr, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        __io_putchar(*ptr++);
    }

    return len;
}

#else
#warning Not supported compiler type
#endif

/**@} end of group Template_Functions */
/**@} end of group Template */
/**@} end of group Examples */
