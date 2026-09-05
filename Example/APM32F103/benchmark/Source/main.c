/*!
 * @file        main.c
 *
 * @brief       LittleFS Performance Benchmark Demo
 *
 * @version     V1.0.0
 *
 * @date        2026-08-16
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
#include "board_delay.h"

void USART_Init(void);
static int benchmark_test(LFS_Storage_t storage);

/* Test configuration */
#define BENCHMARK_FILE          "/benchmark.dat"
#define SEQ_TEST_SIZE           (32 * 1024)     /* 32KB for sequential test */
#define SEQ_BUFFER_SIZE         512             /* 512 bytes per write */
#define RANDOM_TEST_COUNT       100             /* 100 random operations */
#define RANDOM_BUFFER_SIZE      64              /* 64 bytes per random op */
#define FILE_CREATE_COUNT       20              /* Create 20 files */

/** @addtogroup Examples
  @{
  */

/** @addtogroup Benchmark
  @{
  */

/** @defgroup Benchmark_Functions
  @{
  */

/*!
 * @brief       Get system tick count in milliseconds
 *
 * @param       None
 *
 * @retval      Tick count in ms
 */
static uint32_t get_tick_ms(void)
{
    return BOARD_ReadTick();
}

/*!
 * @brief       Calculate throughput in KB/s
 *
 * @param       bytes: Total bytes transferred
 * @param       time_ms: Time in milliseconds
 *
 * @retval      Throughput in KB/s (x100 for precision)
 */
static uint32_t calc_throughput(uint32_t bytes, uint32_t time_ms)
{
    if (time_ms == 0) return 0;
    return (bytes * 1000 * 100) / (time_ms * 1024);
}

/*!
 * @brief       Main program
 *
 * @param       None
 *
 * @retval      None
 */
int main(void)
{
    /* System clocks configuration */
    SystemClockConfig();
    USART_Init();
    
    /* Initialize delay counter */
    BOARD_DelayConfig();

    printf("\n=== LittleFS Performance Benchmark Demo ===\n");
    printf("Flash Area: 0x08010000 - 0x0801FFFF (64KB)\n");
    printf("Block Size: %dKB, Block Count: %d\n\n", LFS_INT_BLOCK_SIZE/1024, LFS_INT_BLOCK_COUNT);

    LFS_App_Init(LFS_STORAGE_INTERNAL);

    /* Run benchmark test */
    benchmark_test(LFS_STORAGE_INTERNAL);

    printf("\n=== Benchmark Complete ===\n");

    while(1)
    {
    }
}

/*!
 * @brief       Performance benchmark test
 *
 * @param       storage: Storage type (internal/external flash)
 *
 * @retval      0 on success, error code on failure
 */
static int benchmark_test(LFS_Storage_t storage)
{
    int err;
    uint32_t start_time, end_time;
    uint32_t i;
    uint8_t buffer[SEQ_BUFFER_SIZE];
    uint32_t errors = 0;
    uint32_t seq_write_time = 0, seq_read_time = 0;
    uint32_t rand_write_time = 0, rand_read_time = 0;

    printf("\n=== Starting Benchmark Tests ===\n");

    /* Mount filesystem */
    err = LFS_App_Mount(storage);
    if (err != LFS_ERR_OK)
    {
        printf("Mount Failed: %d (0x%04X)\n", err, (uint16_t)err);
        return err;
    }
    printf("Filesystem mounted successfully\n");

    /* Initialize test buffer */
    for (i = 0; i < sizeof(buffer); i++)
    {
        buffer[i] = (uint8_t)(i & 0xFF);
    }

    /* Test 1: Sequential Write Speed */
    printf("\n[Test 1] Sequential Write Speed\n");
    printf("  Writing %d bytes in %d-byte chunks...\n", SEQ_TEST_SIZE, SEQ_BUFFER_SIZE);
    
    err = LFS_App_OpenFile(storage, BENCHMARK_FILE, LFS_MODE_WRITE);
    if (err != LFS_ERR_OK)
    {
        printf("  Failed to open file: %d\n", err);
        errors++;
    }
    else
    {
        uint32_t chunks = SEQ_TEST_SIZE / SEQ_BUFFER_SIZE;
        start_time = get_tick_ms();
        
        for (i = 0; i < chunks; i++)
        {
            err = LFS_App_WriteFile(storage, buffer, SEQ_BUFFER_SIZE);
            if (err < 0)
            {
                printf("  Write failed at chunk %ld: %d\n", i, err);
                errors++;
                break;
            }
        }
        
        end_time = get_tick_ms();
        LFS_App_CloseFile(storage);

        seq_write_time = end_time - start_time;
        uint32_t throughput = calc_throughput(SEQ_TEST_SIZE, seq_write_time);

        printf("  Completed in %ld ms\n", seq_write_time);
        printf("  Throughput: %ld.%02ld KB/s\n", throughput / 100, throughput % 100);
    }

    /* Test 2: Sequential Read Speed */
    printf("\n[Test 2] Sequential Read Speed\n");
    printf("  Reading %d bytes in %d-byte chunks...\n", SEQ_TEST_SIZE, SEQ_BUFFER_SIZE);
    
    err = LFS_App_OpenFile(storage, BENCHMARK_FILE, LFS_MODE_READ);
    if (err != LFS_ERR_OK)
    {
        printf("  Failed to open file: %d\n", err);
        errors++;
    }
    else
    {
        uint32_t chunks = SEQ_TEST_SIZE / SEQ_BUFFER_SIZE;
        uint8_t read_buffer[SEQ_BUFFER_SIZE];
        start_time = get_tick_ms();
        
        for (i = 0; i < chunks; i++)
        {
            err = LFS_App_ReadFile(storage, read_buffer, SEQ_BUFFER_SIZE);
            if (err < 0)
            {
                printf("  Read failed at chunk %ld: %d\n", i, err);
                errors++;
                break;
            }
        }
        
        end_time = get_tick_ms();
        LFS_App_CloseFile(storage);

        seq_read_time = end_time - start_time;
        uint32_t throughput = calc_throughput(SEQ_TEST_SIZE, seq_read_time);

        printf("  Completed in %ld ms\n", seq_read_time);
        printf("  Throughput: %ld.%02ld KB/s\n", throughput / 100, throughput % 100);
    }

    /* Test 3: Random Write Speed */
    printf("\n[Test 3] Random Write Speed\n");
    printf("  Performing %d random writes of %d bytes...\n", RANDOM_TEST_COUNT, RANDOM_BUFFER_SIZE);
    
    err = LFS_App_OpenFile(storage, BENCHMARK_FILE, LFS_MODE_WRITE);
    if (err != LFS_ERR_OK)
    {
        printf("  Failed to open file: %d\n", err);
        errors++;
    }
    else
    {
        uint8_t rand_buffer[RANDOM_BUFFER_SIZE];
        start_time = get_tick_ms();
        
        for (i = 0; i < RANDOM_TEST_COUNT; i++)
        {
            /* Fill with pseudo-random data */
            uint32_t j;
            for (j = 0; j < sizeof(rand_buffer); j++)
            {
                rand_buffer[j] = (uint8_t)((i * 31 + j * 17) & 0xFF);
            }
            
            /* Seek to random position */
            uint32_t pos = (i * 137) % (SEQ_TEST_SIZE - RANDOM_BUFFER_SIZE);
            err = LFS_App_SeekFile(storage, pos, LFS_SEEK_SET);
            if (err < 0)
            {
                printf("  Seek failed at iteration %ld: %d\n", i, err);
                errors++;
                break;
            }
            
            err = LFS_App_WriteFile(storage, rand_buffer, RANDOM_BUFFER_SIZE);
            if (err < 0)
            {
                printf("  Write failed at iteration %ld: %d\n", i, err);
                errors++;
                break;
            }
        }
        
        end_time = get_tick_ms();
        LFS_App_CloseFile(storage);

        rand_write_time = end_time - start_time;
        uint32_t total_bytes = RANDOM_TEST_COUNT * RANDOM_BUFFER_SIZE;
        uint32_t throughput = calc_throughput(total_bytes, rand_write_time);

        printf("  Completed in %ld ms\n", rand_write_time);
        printf("  Throughput: %ld.%02ld KB/s\n", throughput / 100, throughput % 100);
        printf("  Avg time per operation: %ld us\n", (rand_write_time * 1000) / RANDOM_TEST_COUNT);
    }

    /* Test 4: Random Read Speed */
    printf("\n[Test 4] Random Read Speed\n");
    printf("  Performing %d random reads of %d bytes...\n", RANDOM_TEST_COUNT, RANDOM_BUFFER_SIZE);
    
    err = LFS_App_OpenFile(storage, BENCHMARK_FILE, LFS_MODE_READ);
    if (err != LFS_ERR_OK)
    {
        printf("  Failed to open file: %d\n", err);
        errors++;
    }
    else
    {
        uint8_t read_buffer[RANDOM_BUFFER_SIZE];
        start_time = get_tick_ms();
        
        for (i = 0; i < RANDOM_TEST_COUNT; i++)
        {
            /* Seek to random position */
            uint32_t pos = (i * 137) % (SEQ_TEST_SIZE - RANDOM_BUFFER_SIZE);
            err = LFS_App_SeekFile(storage, pos, LFS_SEEK_SET);
            if (err < 0)
            {
                printf("  Seek failed at iteration %ld: %d\n", i, err);
                errors++;
                break;
            }
            
            err = LFS_App_ReadFile(storage, read_buffer, RANDOM_BUFFER_SIZE);
            if (err < 0)
            {
                printf("  Read failed at iteration %ld: %d\n", i, err);
                errors++;
                break;
            }
        }
        
        end_time = get_tick_ms();
        LFS_App_CloseFile(storage);

        rand_read_time = end_time - start_time;
        uint32_t total_bytes = RANDOM_TEST_COUNT * RANDOM_BUFFER_SIZE;
        uint32_t throughput = calc_throughput(total_bytes, rand_read_time);

        printf("  Completed in %ld ms\n", rand_read_time);
        printf("  Throughput: %ld.%02ld KB/s\n", throughput / 100, throughput % 100);
        printf("  Avg time per operation: %ld us\n", (rand_read_time * 1000) / RANDOM_TEST_COUNT);
    }

    /* Test 5: File Creation/Deletion Speed */
    printf("\n[Test 5] File Create/Delete Speed\n");
    printf("  Creating %d files...\n", FILE_CREATE_COUNT);
    
    char path[32];
    start_time = get_tick_ms();
    
    for (i = 0; i < FILE_CREATE_COUNT; i++)
    {
        snprintf(path, sizeof(path), "/test_%ld.dat", i);
        err = LFS_App_OpenFile(storage, path, LFS_MODE_WRITE);
        if (err == LFS_ERR_OK)
        {
            LFS_App_WriteFile(storage, buffer, 64);
            LFS_App_CloseFile(storage);
        }
        else
        {
            printf("  Create failed at file %ld: %d\n", i, err);
            errors++;
        }
    }
    
    end_time = get_tick_ms();
    uint32_t create_time = end_time - start_time;
    printf("  Created %d files in %ld ms\n", FILE_CREATE_COUNT, create_time);
    printf("  Avg time per file: %ld.%02ld ms\n", 
           (create_time * 100) / FILE_CREATE_COUNT / 100,
           ((create_time * 100) / FILE_CREATE_COUNT) % 100);

    printf("  Deleting %d files...\n", FILE_CREATE_COUNT);
    start_time = get_tick_ms();
    
    for (i = 0; i < FILE_CREATE_COUNT; i++)
    {
        snprintf(path, sizeof(path), "/test_%ld.dat", i);
        err = LFS_App_RemoveFile(storage, path);
        if (err != LFS_ERR_OK)
        {
            printf("  Delete failed at file %ld: %d\n", i, err);
            errors++;
        }
    }
    
    end_time = get_tick_ms();
    uint32_t delete_time = end_time - start_time;
    printf("  Deleted %d files in %ld ms\n", FILE_CREATE_COUNT, delete_time);
    printf("  Avg time per file: %ld.%02ld ms\n",
           (delete_time * 100) / FILE_CREATE_COUNT / 100,
           ((delete_time * 100) / FILE_CREATE_COUNT) % 100);

    /* Cleanup benchmark file */
    LFS_App_RemoveFile(storage, BENCHMARK_FILE);

    /* Unmount */
    LFS_App_Unmount(storage);

    /* Summary */
    printf("\n=== Benchmark Summary ===\n");
    printf("Sequential Write: %ld.%02ld KB/s\n",
           calc_throughput(SEQ_TEST_SIZE, seq_write_time) / 100,
           calc_throughput(SEQ_TEST_SIZE, seq_write_time) % 100);
    printf("Sequential Read: %ld.%02ld KB/s\n",
           calc_throughput(SEQ_TEST_SIZE, seq_read_time) / 100,
           calc_throughput(SEQ_TEST_SIZE, seq_read_time) % 100);
    printf("Random Write: %ld.%02ld KB/s\n",
           calc_throughput(RANDOM_TEST_COUNT * RANDOM_BUFFER_SIZE, rand_write_time) / 100,
           calc_throughput(RANDOM_TEST_COUNT * RANDOM_BUFFER_SIZE, rand_write_time) % 100);
    printf("Random Read: %ld.%02ld KB/s\n",
           calc_throughput(RANDOM_TEST_COUNT * RANDOM_BUFFER_SIZE, rand_read_time) / 100,
           calc_throughput(RANDOM_TEST_COUNT * RANDOM_BUFFER_SIZE, rand_read_time) % 100);
    printf("File Create: %ld.%02ld ms/file\n",
           (create_time * 100) / FILE_CREATE_COUNT / 100,
           ((create_time * 100) / FILE_CREATE_COUNT) % 100);
    printf("File Delete: %ld.%02ld ms/file\n",
           (delete_time * 100) / FILE_CREATE_COUNT / 100,
           ((delete_time * 100) / FILE_CREATE_COUNT) % 100);
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

/**@} end of group Benchmark_Functions */
/**@} end of group Benchmark */
/**@} end of group Examples */
