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
static int param_persistence_test(LFS_Storage_t storage);

/** @addtogroup Examples
  @{
  */

/** @addtogroup Template
  @{
  */

/** @defgroup Template_Functions
  @{
  */

/* Device configuration parameter structure */
typedef struct {
    uint32_t magic;           /* Magic number for validation */
    uint32_t boot_count;      /* Boot counter */
    uint32_t device_id;       /* Device unique ID */
    int32_t  calibration;     /* Calibration offset */
    uint16_t voltage_mv;      /* Operating voltage in mV */
    uint16_t reserved;        /* Reserved for alignment */
    char     device_name[32]; /* Device name string */
    uint32_t crc32;           /* CRC32 checksum */
} DeviceParams_t;

#define PARAM_MAGIC_NUMBER    0xDEADBEEF
#define PARAM_FILE_PATH       "/device_params.bin"

/*!
 * @brief       Calculate CRC32 checksum
 *
 * @param       data: Pointer to data
 * @param       len:  Data length in bytes
 *
 * @retval      CRC32 value
 */
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

/*!
 * @brief       Initialize default parameters
 *
 * @param       params: Pointer to parameter structure
 *
 * @retval      None
 */
static void params_set_defaults(DeviceParams_t *params)
{
    memset(params, 0, sizeof(DeviceParams_t));
    params->magic = PARAM_MAGIC_NUMBER;
    params->boot_count = 0;
    params->device_id = 0x00A0F103;
    params->calibration = -50;
    params->voltage_mv = 3300;
    params->reserved = 0;
    snprintf(params->device_name, sizeof(params->device_name), "APM32F103VB");
    params->crc32 = calc_crc32((const uint8_t *)params,
                               sizeof(DeviceParams_t) - sizeof(uint32_t));
}

/*!
 * @brief       Load parameters from flash
 *
 * @param       storage: Storage device type
 * @param       params:  Pointer to parameter structure
 *
 * @retval      0 on success, negative on failure
 */
static int params_load(LFS_Storage_t storage, DeviceParams_t *params)
{
    int err;

    err = LFS_App_OpenFile(storage, PARAM_FILE_PATH, LFS_MODE_READ);
    if (err != LFS_ERR_OK)
    {
        return err;
    }

    err = LFS_App_ReadFile(storage, params, sizeof(DeviceParams_t));
    LFS_App_CloseFile(storage);

    if (err < 0)
    {
        return err;
    }

    /* Validate magic number */
    if (params->magic != PARAM_MAGIC_NUMBER)
    {
        printf("  Invalid magic number: 0x%08lX\n", params->magic);
        return LFS_ERR_CORRUPT;
    }

    /* Validate CRC32 */
    uint32_t expected_crc = calc_crc32((const uint8_t *)params,
                                       sizeof(DeviceParams_t) - sizeof(uint32_t));
    if (params->crc32 != expected_crc)
    {
        printf("  CRC mismatch: stored=0x%08lX, calc=0x%08lX\n",
               params->crc32, expected_crc);
        return LFS_ERR_CORRUPT;
    }

    return LFS_ERR_OK;
}

/*!
 * @brief       Save parameters to flash
 *
 * @param       storage: Storage device type
 * @param       params:  Pointer to parameter structure
 *
 * @retval      0 on success, negative on failure
 */
static int params_save(LFS_Storage_t storage, DeviceParams_t *params)
{
    int err;

    /* Recalculate CRC before saving */
    params->crc32 = calc_crc32((const uint8_t *)params,
                               sizeof(DeviceParams_t) - sizeof(uint32_t));

    err = LFS_App_OpenFile(storage, PARAM_FILE_PATH, LFS_MODE_WRITE);
    if (err != LFS_ERR_OK)
    {
        return err;
    }

    err = LFS_App_WriteFile(storage, params, sizeof(DeviceParams_t));
    LFS_App_CloseFile(storage);

    return (err >= 0) ? LFS_ERR_OK : err;
}

/*!
 * @brief       Print current parameters
 *
 * @param       params: Pointer to parameter structure
 *
 * @retval      None
 */
static void params_print(const DeviceParams_t *params)
{
    printf("  Magic:        0x%08lX\n", params->magic);
    printf("  Boot Count:   %lu\n", params->boot_count);
    printf("  Device ID:    0x%08lX\n", params->device_id);
    printf("  Calibration:  %ld\n", params->calibration);
    printf("  Voltage:      %u mV\n", params->voltage_mv);
    printf("  Device Name:  %s\n", params->device_name);
    printf("  CRC32:        0x%08lX\n", params->crc32);
}

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

    printf("\n=== LittleFS Parameter Persistence Demo ===\n");
    printf("Flash Area: 0x08010000 - 0x0801FFFF (64KB)\n");
    printf("Block Size: %dKB, Block Count: %d\n\n", LFS_INT_BLOCK_SIZE/1024, LFS_INT_BLOCK_COUNT);

    LFS_App_Init(LFS_STORAGE_INTERNAL);

    /* Run parameter persistence test */
    param_persistence_test(LFS_STORAGE_INTERNAL);

    printf("\n=== Demo Complete ===\n");

    while(1)
    {
    }
}

static int param_persistence_test(LFS_Storage_t storage)
{
    DeviceParams_t params;
    int err;
    uint32_t errors = 0;

    /* Mount filesystem */
    err = LFS_App_Mount(storage);
    if (err != LFS_ERR_OK)
    {
        printf("Mount Failed: %d (0x%04X)\n", err, (uint16_t)err);
        return err;
    }

    /* Test 1: Load or initialize parameters */
    printf("[Test 1] Load parameters from flash\n");
    err = params_load(storage, &params);
    if (err == LFS_ERR_OK)
    {
        printf("  Parameters loaded successfully:\n");
        params_print(&params);
    }
    else
    {
        printf("  No valid parameters found (err=%d), initializing defaults\n", err);
        params_set_defaults(&params);
        printf("  Default parameters:\n");
        params_print(&params);

        err = params_save(storage, &params);
        if (err == LFS_ERR_OK)
        {
            printf("  Default parameters saved to flash\n");
        }
        else
        {
            printf("  Save failed: %d\n", err);
            errors++;
        }
    }

    /* Test 2: Simulate boot - increment boot counter and persist */
    printf("\n[Test 2] Simulate boot cycle (increment & persist)\n");
    params.boot_count++;
    params.voltage_mv = 3300 + (params.boot_count % 10) * 10;
    params.calibration = -50 + (int32_t)(params.boot_count % 20);

    err = params_save(storage, &params);
    if (err == LFS_ERR_OK)
    {
        printf("  Updated parameters saved (boot #%lu)\n", params.boot_count);
    }
    else
    {
        printf("  Save failed: %d\n", err);
        errors++;
    }

    /* Test 3: Read back and verify data integrity */
    printf("\n[Test 3] Read back and verify data integrity\n");
    DeviceParams_t verify;
    err = params_load(storage, &verify);
    if (err == LFS_ERR_OK)
    {
        if (memcmp(&params, &verify, sizeof(DeviceParams_t)) == 0)
        {
            printf("  PASS: Read-back data matches written data\n");
        }
        else
        {
            printf("  FAIL: Data mismatch!\n");
            errors++;
        }
    }
    else
    {
        printf("  FAIL: Read-back failed: %d\n", err);
        errors++;
    }

    /* Test 4: Simulate multiple boot cycles */
    printf("\n[Test 4] Simulate 10 boot cycles\n");
    uint32_t i;
    for (i = 0; i < 10; i++)
    {
        err = params_load(storage, &params);
        if (err != LFS_ERR_OK)
        {
            printf("  Cycle %lu: Load failed: %d\n", i, err);
            errors++;
            break;
        }

        params.boot_count++;
        params.calibration += 1;

        err = params_save(storage, &params);
        if (err != LFS_ERR_OK)
        {
            printf("  Cycle %lu: Save failed: %d\n", i, err);
            errors++;
            break;
        }
    }
    if (i == 10)
    {
        printf("  10 boot cycles completed successfully\n");
        printf("  Final boot count: %lu\n", params.boot_count);
    }

    /* Test 5: Final verification after all cycles */
    printf("\n[Test 5] Final verification\n");
    DeviceParams_t final_params;
    err = params_load(storage, &final_params);
    if (err == LFS_ERR_OK)
    {
        printf("  Final parameters:\n");
        params_print(&final_params);

        if (final_params.magic == PARAM_MAGIC_NUMBER &&
            final_params.boot_count == params.boot_count)
        {
            printf("  PASS: Parameters persisted correctly\n");
        }
        else
        {
            printf("  FAIL: Parameter validation failed\n");
            errors++;
        }
    }
    else
    {
        printf("  FAIL: Final read-back failed: %d\n", err);
        errors++;
    }

    /* Unmount */
    LFS_App_Unmount(storage);

    printf("\n=== Parameter Persistence Summary ===\n");
    printf("Errors: %lu\n", errors);
    printf("Result: %s\n", (errors == 0) ? "PASS" : "FAIL");
    printf("\nNote: Reset the board to see boot_count increment,\n");
    printf("      demonstrating parameter persistence across power cycles.\n");

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
