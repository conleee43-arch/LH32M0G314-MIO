/******************************************************************************
 * 版权所有：苏州领慧立芯科技有限公司
 * Copyright (c) 2020-2025 Suzhou Legendsemi Technology Co., Ltd.
 ******************************************************************************
 * All rights reserved. Distributed under MIT license.
 * The file is encoded in UTF-8 without signature.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "lh32m0xx_lhl.h"

/* Private define ------------------------------------------------------------*/
#define TEMPLATE_LED_PORT             pGPIO0
#define TEMPLATE_LED_PIN              GPIO_PIN_0
#define TEMPLATE_UART_PORT            pGPIO0
#define TEMPLATE_UART_RX_PIN          GPIO_PIN_1
#define TEMPLATE_UART_TX_PIN          GPIO_PIN_2
#define TEMPLATE_HEARTBEAT_PERIOD_US  1000000UL

/* Private declaration -------------------------------------------------------*/
void SystemClock_Config(void);
void GPIO_Config(void);
void UART0_Config(void);

/* Functions -----------------------------------------------------------------*/
int main(void)
{
    uint32_t heartbeat = 0U;

    /* 上电后IWDG默认开启，若无处理约20秒后复位 */
    LHL_IWDG_Stop();

    /* 1. 配置系统时钟到4MHz，给GPIO/UART提供稳定时钟 */
    SystemClock_Config();

    /* 2. 配置模板默认的LED输出引脚 */
    GPIO_Config();

    /* 3. 配置UART0，并把printf输出重定向到串口 */
    UART0_Config();

    printf("\r\nTemplate_Minimal booted.\r\n");
    printf("Hello World!\r\n");
    printf("LED=P0.0, UART0_TX=P0.2, UART0_RX=P0.1, HCLK=4MHz\r\n");


    while(1)
    {
        LHL_Delay_uS(TEMPLATE_HEARTBEAT_PERIOD_US);
        LHL_GPIO_TogglePin(TEMPLATE_LED_PORT, TEMPLATE_LED_PIN);

        heartbeat++;
        printf("Heartbeat %lu\r\n", (unsigned long)heartbeat);
    }
}


/* 使用内部HSI时钟，将HCLK/APB时钟统一配置到4MHz */
void SystemClock_Config(void)
{
    RCC_ClockInitTypeDef Clock_InitStructure;

    Clock_InitStructure.SYSCLKSource = RCC_SYSCLK_SRC_HSI_DIV2; /* 32MHz HSI / 2 = 16MHz */
    Clock_InitStructure.AHBCLKDivider = RCC_SYSCLK_DIV4;        /* HCLK = 16MHz / 4 = 4MHz */
    Clock_InitStructure.APB1CLKDivider = RCC_HCLK_DIV1;         /* PCLK1 = HCLK = 4MHz */
    Clock_InitStructure.APB2CLKDivider = RCC_HCLK_DIV1;         /* PCLK2 = HCLK = 4MHz */

    if (LHL_RCC_ClockConfig(&Clock_InitStructure, LHL_NRM_TIMEOUT) != LHL_OK)
    {
        while(1)
        {
        }
    }
}

/* 配置模板默认LED引脚：P0.0 */
void GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Pin = TEMPLATE_LED_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Current = GPIO_CURRENT_4mA;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.SchmittTrigger = DISABLE;
    GPIO_InitStructure.Alternate = GPIOx_AF_GPIO;

    LHL_GPIO_Init(TEMPLATE_LED_PORT, &GPIO_InitStructure);
}

/* 配置UART0：P0.1为RX，P0.2为TX，波特率115200-8N1 */
void UART0_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    UART_InitTypeDef UART_InitStructure;

    GPIO_InitStructure.Pin = TEMPLATE_UART_RX_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Current = GPIO_CURRENT_4mA;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.SchmittTrigger = ENABLE;
    GPIO_InitStructure.Alternate = GPIO0_1_AF_SIN0;
    LHL_GPIO_Init(TEMPLATE_UART_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = TEMPLATE_UART_TX_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Current = GPIO_CURRENT_4mA;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.SchmittTrigger = DISABLE;
    GPIO_InitStructure.Alternate = GPIO0_2_AF_SOUT0;
    LHL_GPIO_Init(TEMPLATE_UART_PORT, &GPIO_InitStructure);

    UART_InitStructure.UART_BaudRate = 115200;
    UART_InitStructure.UART_WordLength = UART_WordLength_8b;
    UART_InitStructure.UART_StopBits = UART_StopBits_1;
    UART_InitStructure.UART_Parity = UART_Parity_None;
    LHL_UART_Init(pUART0, &UART_InitStructure);
}

/*********************************End of File**********************************/
