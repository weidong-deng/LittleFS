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
static int circular_log_demo(LFS_Storage_t storage, const char *name);

#define LOG_FILE_PATH           "/circular.log"
#define LOG_MAX_SIZE            2048    /* Maximum log file size in bytes */
#define LOG_ENTRY_COUNT         30      /* Number of log entries to write */

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

    printf("\n=== LittleFS Circular Log Demo ===\n");
    printf("Flash Area: 0x08010000 - 0x0801FFFF (64KB)\n");
    printf("Block Size: %dKB, Block Count: %d\n\n", LFS_INT_BLOCK_SIZE/1024, LFS_INT_BLOCK_COUNT);

    LFS_App_Init(LFS_STORAGE_INTERNAL);

    /* Run circular log demo */
    circular_log_demo(LFS_STORAGE_INTERNAL, "Internal Flash");

    printf("\n=== Demo Complete ===\n");

    while(1)
    {
    }
}

static int circular_log_demo(LFS_Storage_t storage, const char *name)
{
    printf("\n=== %s Circular Log Demo ===\n", name);

    int err = LFS_App_Mount(storage);
    if (err != LFS_ERR_OK)
    {
        printf("Mount Failed: %d (0x%04X)\n", err, (uint16_t)err);
        return err;
    }

    uint32_t errors = 0;
    uint32_t i;
    char log_entry[128];
    uint32_t file_size;

    /* Test 1: Initialize log file */
    printf("\n[Test 1] Initialize log file\n");
    err = LFS_App_OpenFile(storage, LOG_FILE_PATH, LFS_MODE_WRITE);
    if (err != LFS_ERR_OK)
    {
        printf("  Failed to create log file: %d\n", err);
        errors++;
    }
    else
    {
        printf("  Log file created: %s\n", LOG_FILE_PATH);
        LFS_App_CloseFile(storage);
    }

    /* Test 2: Write log entries (simulating circular buffer) */
    printf("\n[Test 2] Write %d log entries\n", LOG_ENTRY_COUNT);
    for (i = 0; i < LOG_ENTRY_COUNT; i++)
    {
        /* Format log entry */
        snprintf(log_entry, sizeof(log_entry),
                 "[LOG #%02ld] System event: sensor_value=%ld, timestamp=%ld\n",
                 i, 1000 + i * 10, i * 100);

        /* Open file in append mode */
        err = LFS_App_OpenFile(storage, LOG_FILE_PATH, LFS_MODE_APPEND);
        if (err != LFS_ERR_OK)
        {
            printf("  Entry %ld: Open failed: %d\n", i, err);
            errors++;
            continue;
        }

        /* Write log entry */
        err = LFS_App_WriteFile(storage, log_entry, strlen(log_entry));
        if (err < 0)
        {
            printf("  Entry %ld: Write failed: %d\n", i, err);
            errors++;
            LFS_App_CloseFile(storage);
            continue;
        }

        LFS_App_CloseFile(storage);

        /* Check file size and simulate circular behavior */
        err = LFS_App_GetFileSize(storage, LOG_FILE_PATH, &file_size);
        if (err == LFS_ERR_OK && file_size > LOG_MAX_SIZE)
        {
            printf("  Log file exceeded max size (%ld > %d), truncating...\n",
                   file_size, LOG_MAX_SIZE);
            /* In a real circular log, you would keep only the last N entries */
            /* For demo, we just truncate and start over */
            err = LFS_App_OpenFile(storage, LOG_FILE_PATH, LFS_MODE_WRITE);
            if (err == LFS_ERR_OK)
            {
                LFS_App_CloseFile(storage);
                printf("  Log file truncated\n");
            }
        }

        /* Progress indicator */
        if ((i + 1) % 10 == 0)
        {
            printf("  Progress: %ld/%d entries written\n", i + 1, LOG_ENTRY_COUNT);
        }
    }
    printf("  Test 2 complete: %d entries written\n", LOG_ENTRY_COUNT);

    /* Test 3: Read and display log file */
    printf("\n[Test 3] Read log file contents\n");
    err = LFS_App_OpenFile(storage, LOG_FILE_PATH, LFS_MODE_READ);
    if (err == LFS_ERR_OK)
    {
        uint8_t read_buffer[256];
        int32_t bytes_read;
        uint32_t total_read = 0;

        printf("  --- Log File Contents ---\n");
        while (1)
        {
            bytes_read = LFS_App_ReadFile(storage, read_buffer, sizeof(read_buffer) - 1);
            if (bytes_read <= 0)
                break;

            read_buffer[bytes_read] = '\0';
            printf("%s", (char *)read_buffer);
            total_read += bytes_read;
        }
        printf("  --- End of Log File ---\n");
        printf("  Total bytes read: %ld\n", total_read);

        LFS_App_CloseFile(storage);
    }
    else
    {
        printf("  Failed to open log file for reading: %d\n", err);
        errors++;
    }

    /* Test 4: File system statistics */
    printf("\n[Test 4] File system statistics\n");
    struct lfs_fsinfo fsinfo;
    err = LFS_App_GetFSInfo(storage, &fsinfo);
    if (err == LFS_ERR_OK)
    {
        printf("  Disk version: %ld.%ld\n",
               (fsinfo.disk_version >> 16) & 0xFFFF,
               fsinfo.disk_version & 0xFFFF);
        printf("  Block size: %ld bytes\n", fsinfo.block_size);
        printf("  Block count: %ld\n", fsinfo.block_count);
    }

    int32_t fs_size = LFS_App_GetFSSize(storage);
    if (fs_size >= 0)
    {
        printf("  Used blocks: %ld\n", fs_size);
        printf("  Available blocks: %ld\n", fsinfo.block_count - fs_size);
        printf("  Usage: %.1f%%\n", (float)fs_size / fsinfo.block_count * 100);
    }

    /* Test 5: Get log file size */
    printf("\n[Test 5] Log file information\n");
    err = LFS_App_GetFileSize(storage, LOG_FILE_PATH, &file_size);
    if (err == LFS_ERR_OK)
    {
        printf("  Log file size: %ld bytes\n", file_size);
        printf("  Max log size: %d bytes\n", LOG_MAX_SIZE);
        printf("  Usage: %.1f%%\n", (float)file_size / LOG_MAX_SIZE * 100);
    }

    /* Cleanup */
    printf("\n[Cleanup] Removing log file\n");
    LFS_App_RemoveFile(storage, LOG_FILE_PATH);

    /* Unmount */
    LFS_App_Unmount(storage);

    printf("\n=== Circular Log Demo Summary ===\n");
    printf("Log entries written: %d\n", LOG_ENTRY_COUNT);
    printf("Errors: %ld\n", errors);
    printf("Result: %s\n", (errors == 0) ? "PASS" : "FAIL");

    return (errors == 0) ? LFS_ERR_OK : LFS_ERR_IO;
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
