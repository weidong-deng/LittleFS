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
static int wear_leveling(LFS_Storage_t storage, const char *name);

#define WEAR_TEST_ITERATIONS    1000

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

    printf("\n=== LittleFS Internal Flash Wear Leveling Demo ===\n");
    printf("Flash Area: 0x08010000 - 0x0801FFFF (64KB)\n");
    printf("Block Size: %dKB, Block Count: %d\n\n", LFS_INT_BLOCK_SIZE/1024, LFS_INT_BLOCK_COUNT);

    LFS_App_Init(LFS_STORAGE_INTERNAL);

    /* Run wear leveling test */
    wear_leveling(LFS_STORAGE_INTERNAL, "Internal Flash");

    printf("\n=== Test Complete ===\n");

    while(1)
    {
    }
}

static int wear_leveling(LFS_Storage_t storage, const char *name)
{
    printf("\n=== %s wear leveling ===\n", name);

    /* Reset erase statistics before test */
    LFS_Int_ResetEraseStats();

    int err = LFS_App_Mount(storage);
    if (err != LFS_ERR_OK)
    {
        printf("Mount Failed: %d (0x%04X)\n", err, (uint16_t)err);
        return err;
    }

    char path[32];
    uint8_t buffer[512];
    uint8_t read_buffer[512];
    uint32_t total_writes = 0;
    uint32_t errors = 0;
    uint32_t i;

    printf("Start writing tests, iterating %d times...\n", WEAR_TEST_ITERATIONS);

    /* Test 1: Repeated write to same file (stress test) */
    printf("\n[Test 1] Repeated write to same file\n");
    for (i = 0; i < WEAR_TEST_ITERATIONS; i++)
    {
        /* Generate unique pattern for each iteration */
        memset(buffer, (uint8_t)(i & 0xFF), sizeof(buffer));

        /* Open file for writing */
        err = LFS_App_OpenFile(storage, "/wear_test.dat", LFS_MODE_WRITE);
        if (err != LFS_ERR_OK)
        {
            printf("  Iteration %ld: Open failed: %d (0x%04X)\n", i, err, (uint16_t)err);
            errors++;
            continue;
        }

        /* Write data */
        err = LFS_App_WriteFile(storage, buffer, sizeof(buffer));
        if (err < 0)
        {
            printf("  Iteration %ld: Write failed: %d (0x%04X)\n", i, err, (uint16_t)err);
            errors++;
            LFS_App_CloseFile(storage);
            continue;
        }
        total_writes++;

        LFS_App_CloseFile(storage);

        /* Verify by reading back */
        err = LFS_App_OpenFile(storage, "/wear_test.dat", LFS_MODE_READ);
        if (err == LFS_ERR_OK)
        {
            memset(read_buffer, 0, sizeof(read_buffer));
            err = LFS_App_ReadFile(storage, read_buffer, sizeof(read_buffer));
            if (err < 0)
            {
                printf("  Iteration %ld: Read failed: %d (0x%04X)\n", i, err, (uint16_t)err);
                errors++;
            }
            else if (memcmp(buffer, read_buffer, sizeof(buffer)) != 0)
            {
                printf("  Iteration %ld: Data mismatch!\n", i);
                errors++;
            }
            LFS_App_CloseFile(storage);
        }

        /* Progress indicator */
        if ((i + 1) % 100 == 0)
        {
            printf("  Progress: %ld/%d iterations\n", i + 1, WEAR_TEST_ITERATIONS);
        }
    }
    printf("  Test 1 complete: %ld writes, %ld errors\n", total_writes, errors);

    /* Test 2: Write to multiple files (spread wear) */
    printf("\n[Test 2] Write to multiple files\n");
    uint32_t multi_writes = 0;
    for (i = 0; i < 10; i++)
    {
        snprintf(path, sizeof(path), "/multi_%ld.dat", i);
        memset(buffer, (uint8_t)(0x50 + i), sizeof(buffer));

        err = LFS_App_OpenFile(storage, path, LFS_MODE_WRITE);
        if (err == LFS_ERR_OK)
        {
            err = LFS_App_WriteFile(storage, buffer, sizeof(buffer));
            if (err > 0) multi_writes++;
            LFS_App_CloseFile(storage);
        }
    }
    printf("  Test 2 complete: %ld files written\n", multi_writes);

    /* Test 3: Directory operations */
    printf("\n[Test 3] Directory operations\n");
    err = LFS_App_MakeDir(storage, "/testdir");
    if (err == LFS_ERR_OK)
    {
        printf("  Created directory: /testdir\n");
        
        /* Create files in directory */
        for (i = 0; i < 5; i++)
        {
            snprintf(path, sizeof(path), "/testdir/file_%ld.dat", i);
            memset(buffer, (uint8_t)(0xA0 + i), sizeof(buffer));
            
            err = LFS_App_OpenFile(storage, path, LFS_MODE_WRITE);
            if (err == LFS_ERR_OK)
            {
                LFS_App_WriteFile(storage, buffer, sizeof(buffer));
                LFS_App_CloseFile(storage);
            }
        }
        
        /* List directory contents */
        printf("  Directory contents:\n");
        LFS_App_ListDir(storage, "/testdir");
    }

    /* Test 4: File system statistics before GC */
    printf("\n[Test 4] File system statistics\n");
    struct lfs_fsinfo fsinfo;
    err = LFS_App_GetFSInfo(storage, &fsinfo);
    if (err == LFS_ERR_OK)
    {
        printf("  Disk version: %ld.%ld\n",
               (fsinfo.disk_version >> 16) & 0xFFFF, 
               fsinfo.disk_version & 0xFFFF);
        printf("  Block size: %ld (0x%lX) bytes\n", fsinfo.block_size, fsinfo.block_size);
        printf("  Block count: %ld (0x%lX)\n", fsinfo.block_count, fsinfo.block_count);
        printf("  [Config] Max file name: %ld bytes\n", fsinfo.name_max);
        printf("  [Config] Max file size: %ld (0x%lX) bytes\n", fsinfo.file_max, fsinfo.file_max);
    }
    
    int32_t fs_size = LFS_App_GetFSSize(storage);
    if (fs_size >= 0)
    {
        printf("  [Actual] Used blocks: %ld (0x%lX)\n", fs_size, fs_size);
        printf("  [Actual] Available blocks: %ld (0x%lX)\n", fsinfo.block_count - fs_size, fsinfo.block_count - fs_size);
        printf("  [Actual] Usage: %.1f%%\n", (float)fs_size / fsinfo.block_count * 100);
    }

    /* Test 5: Garbage collection */
    printf("\n[Test 5] Garbage collection\n");
    err = LFS_App_FSGC(storage);
    if (err == LFS_ERR_OK)
    {
        printf("  GC completed successfully\n");
        
        fs_size = LFS_App_GetFSSize(storage);
        if (fs_size >= 0)
        {
            printf("  Used blocks after GC: %ld\n", fs_size);
        }
    }
    else
    {
        printf("  GC failed: %d\n", err);
    }

    /* Test 6: File rename operation */
    printf("\n[Test 6] File rename\n");
    err = LFS_App_Rename(storage, "/wear_test.dat", "/wear_test_renamed.dat");
    if (err == LFS_ERR_OK)
    {
        printf("  Renamed /wear_test.dat -> /wear_test_renamed.dat\n");
        
        /* Verify renamed file exists */
        if (LFS_App_FileExists(storage, "/wear_test_renamed.dat"))
        {
            printf("  Verified: renamed file exists\n");
        }
    }

    /* Cleanup test files */
    printf("\n[Cleanup] Removing test files\n");
    LFS_App_RemoveFile(storage, "/wear_test.dat");
    LFS_App_RemoveFile(storage, "/wear_test_renamed.dat");
    for (i = 0; i < 10; i++)
    {
        snprintf(path, sizeof(path), "/multi_%ld.dat", i);
        LFS_App_RemoveFile(storage, path);
    }
    
    /* Remove directory and its contents */
    for (i = 0; i < 5; i++)
    {
        snprintf(path, sizeof(path), "/testdir/file_%ld.dat", i);
        LFS_App_RemoveFile(storage, path);
    }
    LFS_App_RemoveDir(storage, "/testdir");

    /* Unmount */
    LFS_App_Unmount(storage);

    printf("\n=== Wear Leveling Test Summary ===\n");
    printf("Total iterations: %d\n", WEAR_TEST_ITERATIONS);
    printf("Total writes: %ld\n", total_writes + multi_writes);
    printf("Errors: %ld\n", errors);
    printf("Result: %s\n", (errors == 0) ? "PASS" : "FAIL");

    /* Print erase statistics to verify wear leveling */
    LFS_Int_PrintEraseStats();

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
