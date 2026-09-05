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
#include "shell.h"

void USART_Init(void);
static void USART1_IntConfig(void);

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
    USART1_IntConfig();

    printf("\n=== LittleFS Interactive Shell Demo ===\n");
    printf("Flash Area: 0x08010000 - 0x0801FFFF (64KB)\n");
    printf("Block Size: %dKB, Block Count: %d\n", LFS_INT_BLOCK_SIZE/1024, LFS_INT_BLOCK_COUNT);
    printf("Type 'help' for available commands\n\n");

    /* Initialize shell and mount filesystem */
    Shell_Init(LFS_STORAGE_INTERNAL);
    Shell_PrintPrompt();

    /* Main loop - process shell input */
    while(1)
    {
        Shell_Task();
    }
}

/*!
 * @brief       Configure USART1 interrupt for RX
 *
 * @param       None
 *
 * @retval      None
 */
static void USART1_IntConfig(void)
{
    /* Enable USART1 RXDNE interrupt */
    USART_EnableInterrupt(USART1, USART_INT_RXBNE);

    /* Configure NVIC for USART1 */
    NVIC_EnableIRQRequest(USART1_IRQn, 0, 0);
}

/* wear_leveling function removed - now using interactive shell */

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
