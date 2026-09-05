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
static int multi_file_demo(LFS_Storage_t storage, const char *name);

#define MAX_PATH_LEN    64
#define MAX_FILES       5

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

    printf("\n=== LittleFS Multi-File & Directory Demo ===\n");
    printf("Flash Area: 0x08010000 - 0x0801FFFF (64KB)\n");
    printf("Block Size: %dKB, Block Count: %d\n\n", LFS_INT_BLOCK_SIZE/1024, LFS_INT_BLOCK_COUNT);

    LFS_App_Init(LFS_STORAGE_INTERNAL);

    /* Run multi-file demo */
    multi_file_demo(LFS_STORAGE_INTERNAL, "Internal Flash");

    printf("\n=== Demo Complete ===\n");

    while(1)
    {
    }
}

static int multi_file_demo(LFS_Storage_t storage, const char *name)
{
    printf("\n=== %s Multi-File & Directory Demo ===\n", name);

    int err = LFS_App_Mount(storage);
    if (err != LFS_ERR_OK)
    {
        printf("Mount Failed: %d (0x%04X)\n", err, (uint16_t)err);
        return err;
    }

    char path[MAX_PATH_LEN];
    uint8_t buffer[256];
    uint8_t read_buffer[256];
    uint32_t errors = 0;
    uint32_t i;

    /* Test 1: Create multi-level directory structure */
    printf("\n[Test 1] Create multi-level directory structure\n");
    
    /* Create root directories */
    err = LFS_App_MakeDir(storage, "/data");
    if (err == LFS_ERR_OK)
    {
        printf("  Created: /data\n");
    }
    else
    {
        printf("  Failed to create /data: %d\n", err);
        errors++;
    }

    err = LFS_App_MakeDir(storage, "/config");
    if (err == LFS_ERR_OK)
    {
        printf("  Created: /config\n");
    }
    else
    {
        printf("  Failed to create /config: %d\n", err);
        errors++;
    }

    /* Create subdirectories */
    err = LFS_App_MakeDir(storage, "/data/logs");
    if (err == LFS_ERR_OK)
    {
        printf("  Created: /data/logs\n");
    }
    else
    {
        printf("  Failed to create /data/logs: %d\n", err);
        errors++;
    }

    err = LFS_App_MakeDir(storage, "/data/sensors");
    if (err == LFS_ERR_OK)
    {
        printf("  Created: /data/sensors\n");
    }
    else
    {
        printf("  Failed to create /data/sensors: %d\n", err);
        errors++;
    }

    /* Test 2: Open and write multiple files simultaneously */
    printf("\n[Test 2] Open and write multiple files\n");
    
    /* Write config file */
    err = LFS_App_OpenFile(storage, "/config/settings.cfg", LFS_MODE_WRITE);
    if (err == LFS_ERR_OK)
    {
        const char *config_data = "device_id=APM32F103\nversion=1.0\nmode=normal\n";
        err = LFS_App_WriteFile(storage, config_data, strlen(config_data));
        if (err >= 0)
        {
            printf("  Written: /config/settings.cfg (%d bytes)\n", err);
        }
        LFS_App_CloseFile(storage);
    }
    else
    {
        printf("  Failed to open /config/settings.cfg: %d\n", err);
        errors++;
    }

    /* Write sensor data files */
    for (i = 0; i < MAX_FILES; i++)
    {
        snprintf(path, sizeof(path), "/data/sensors/sensor_%ld.dat", i);
        memset(buffer, (uint8_t)(0xA0 + i), sizeof(buffer));
        
        err = LFS_App_OpenFile(storage, path, LFS_MODE_WRITE);
        if (err == LFS_ERR_OK)
        {
            err = LFS_App_WriteFile(storage, buffer, sizeof(buffer));
            if (err >= 0)
            {
                printf("  Written: %s (%d bytes)\n", path, err);
            }
            LFS_App_CloseFile(storage);
        }
        else
        {
            printf("  Failed to open %s: %d\n", path, err);
            errors++;
        }
    }

    /* Write log file */
    err = LFS_App_OpenFile(storage, "/data/logs/system.log", LFS_MODE_WRITE);
    if (err == LFS_ERR_OK)
    {
        const char *log_data = "[LOG] System started\n[LOG] All sensors initialized\n[LOG] Ready\n";
        err = LFS_App_WriteFile(storage, log_data, strlen(log_data));
        if (err >= 0)
        {
            printf("  Written: /data/logs/system.log (%d bytes)\n", err);
        }
        LFS_App_CloseFile(storage);
    }
    else
    {
        printf("  Failed to open /data/logs/system.log: %d\n", err);
        errors++;
    }

    /* Test 3: Directory traversal - list all directories */
    printf("\n[Test 3] Directory traversal\n");
    
    printf("  Root directory (/):\n");
    LFS_App_ListDir(storage, "/");
    
    printf("\n  /data directory:\n");
    LFS_App_ListDir(storage, "/data");
    
    printf("\n  /data/sensors directory:\n");
    LFS_App_ListDir(storage, "/data/sensors");
    
    printf("\n  /config directory:\n");
    LFS_App_ListDir(storage, "/config");

    /* Test 4: Read and verify files */
    printf("\n[Test 4] Read and verify files\n");
    
    /* Read config file */
    err = LFS_App_OpenFile(storage, "/config/settings.cfg", LFS_MODE_READ);
    if (err == LFS_ERR_OK)
    {
        memset(read_buffer, 0, sizeof(read_buffer));
        err = LFS_App_ReadFile(storage, read_buffer, sizeof(read_buffer) - 1);
        if (err >= 0)
        {
            printf("  Read /config/settings.cfg:\n    %s\n", read_buffer);
        }
        LFS_App_CloseFile(storage);
    }
    else
    {
        printf("  Failed to read /config/settings.cfg: %d\n", err);
        errors++;
    }

    /* Verify sensor data */
    for (i = 0; i < MAX_FILES; i++)
    {
        snprintf(path, sizeof(path), "/data/sensors/sensor_%ld.dat", i);
        err = LFS_App_OpenFile(storage, path, LFS_MODE_READ);
        if (err == LFS_ERR_OK)
        {
            memset(read_buffer, 0, sizeof(read_buffer));
            err = LFS_App_ReadFile(storage, read_buffer, sizeof(read_buffer));
            if (err >= 0)
            {
                /* Verify pattern */
                int valid = 1;
                uint32_t j;
                for (j = 0; j < sizeof(buffer); j++)
                {
                    if (read_buffer[j] != (uint8_t)(0xA0 + i))
                    {
                        valid = 0;
                        break;
                    }
                }
                if (valid)
                {
                    printf("  Verified: %s (pattern OK)\n", path);
                }
                else
                {
                    printf("  FAIL: %s (pattern mismatch)\n", path);
                    errors++;
                }
            }
            LFS_App_CloseFile(storage);
        }
    }

    /* Test 5: File rename */
    printf("\n[Test 5] File rename\n");
    
    err = LFS_App_Rename(storage, "/data/logs/system.log", "/data/logs/system_old.log");
    if (err == LFS_ERR_OK)
    {
        printf("  Renamed: /data/logs/system.log -> /data/logs/system_old.log\n");
        
        /* Verify new file exists */
        if (LFS_App_FileExists(storage, "/data/logs/system_old.log"))
        {
            printf("  Verified: /data/logs/system_old.log exists\n");
        }
    }
    else
    {
        printf("  Failed to rename: %d\n", err);
        errors++;
    }

    /* Test 6: File move (rename to different directory) */
    printf("\n[Test 6] File move\n");
    
    err = LFS_App_Rename(storage, "/data/sensors/sensor_0.dat", "/config/sensor_backup.dat");
    if (err == LFS_ERR_OK)
    {
        printf("  Moved: /data/sensors/sensor_0.dat -> /config/sensor_backup.dat\n");
        
        /* Verify file exists in new location */
        if (LFS_App_FileExists(storage, "/config/sensor_backup.dat"))
        {
            printf("  Verified: /config/sensor_backup.dat exists\n");
        }
    }
    else
    {
        printf("  Failed to move: %d\n", err);
        errors++;
    }

    /* Test 7: File delete */
    printf("\n[Test 7] File delete\n");
    
    err = LFS_App_RemoveFile(storage, "/data/logs/system_old.log");
    if (err == LFS_ERR_OK)
    {
        printf("  Deleted: /data/logs/system_old.log\n");
        
        /* Verify file is gone */
        if (!LFS_App_FileExists(storage, "/data/logs/system_old.log"))
        {
            printf("  Verified: /data/logs/system_old.log no longer exists\n");
        }
    }
    else
    {
        printf("  Failed to delete: %d\n", err);
        errors++;
    }

    /* Test 8: Final directory listing */
    printf("\n[Test 8] Final directory structure\n");
    
    printf("  Root (/):\n");
    LFS_App_ListDir(storage, "/");
    
    printf("\n  /data:\n");
    LFS_App_ListDir(storage, "/data");
    
    printf("\n  /data/sensors:\n");
    LFS_App_ListDir(storage, "/data/sensors");
    
    printf("\n  /config:\n");
    LFS_App_ListDir(storage, "/config");

    /* Test 9: File system statistics */
    printf("\n[Test 9] File system statistics\n");
    
    struct lfs_fsinfo fsinfo;
    err = LFS_App_GetFSInfo(storage, &fsinfo);
    if (err == LFS_ERR_OK)
    {
        printf("  Disk version: %ld.%ld\n",
               (fsinfo.disk_version >> 16) & 0xFFFF, 
               fsinfo.disk_version & 0xFFFF);
        printf("  Block size: %ld bytes\n", fsinfo.block_size);
        printf("  Block count: %ld\n", fsinfo.block_count);
        printf("  Max file name: %ld bytes\n", fsinfo.name_max);
        printf("  Max file size: %ld bytes\n", fsinfo.file_max);
    }
    
    int32_t fs_size = LFS_App_GetFSSize(storage);
    if (fs_size >= 0)
    {
        printf("  Used blocks: %ld\n", fs_size);
        printf("  Available blocks: %ld\n", fsinfo.block_count - fs_size);
        printf("  Usage: %.1f%%\n", (float)fs_size / fsinfo.block_count * 100);
    }

    /* Cleanup: Remove all test files and directories */
    printf("\n[Cleanup] Removing test files and directories\n");
    
    /* Remove files */
    LFS_App_RemoveFile(storage, "/config/settings.cfg");
    LFS_App_RemoveFile(storage, "/config/sensor_backup.dat");
    for (i = 1; i < MAX_FILES; i++)
    {
        snprintf(path, sizeof(path), "/data/sensors/sensor_%ld.dat", i);
        LFS_App_RemoveFile(storage, path);
    }
    
    /* Remove directories (must be empty) */
    LFS_App_RemoveDir(storage, "/data/logs");
    LFS_App_RemoveDir(storage, "/data/sensors");
    LFS_App_RemoveDir(storage, "/data");
    LFS_App_RemoveDir(storage, "/config");

    /* Unmount */
    LFS_App_Unmount(storage);

    printf("\n=== Multi-File Demo Summary ===\n");
    printf("Directories created: 4\n");
    printf("Files created: 7\n");
    printf("Files renamed: 1\n");
    printf("Files moved: 1\n");
    printf("Files deleted: 1\n");
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
