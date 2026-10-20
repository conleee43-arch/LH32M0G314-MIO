/******************************************************************************
 * 版权所有：苏州领慧立芯科技有限公司
 * Copyright (c) 2020-2025 Suzhou Legendsemi Technology Co., Ltd.
 ******************************************************************************
 * All rights reserved. Distributed under MIT license.
 * The file is encoded in UTF-8 without signature.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "lh32m0xx_lhl.h"

/* Public --------------------------------------------------------------------*/
MIO_UART_HandleTypeDef hMUART;


/* Private define ------------------------------------------------------------*/
void SystemClock_Config(void);


/* Functions -----------------------------------------------------------------*/
int main(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;                    
    uint8_t buff;

    /* 上电后IWDG默认开启，若无处理约20秒后复位 */
	LHL_IWDG_Stop();
    /* 配置时钟树：主频4MHz */
    SystemClock_Config();
    
    /* 1. 初始化MIO和中断用到的GPIO */
    GPIO_InitStructure.Pin = GPIO_PIN_1;                        // P0.1->MIO_D1
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;           
    GPIO_InitStructure.Pull = GPIO_NOPULL;        
    GPIO_InitStructure.SchmittTrigger = ENABLE;                  
    GPIO_InitStructure.Alternate = GPIO0_1_AF_MIO_D1;           // 复用为MIO D1
    LHL_GPIO_Init(pGPIO0, &GPIO_InitStructure);
    
    GPIO_InitStructure.Pin = GPIO_PIN_2;                        // P0.2->MIO_D2
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;           
    GPIO_InitStructure.Current = GPIO_CURRENT_4mA;
    GPIO_InitStructure.Pull = GPIO_NOPULL;        
    GPIO_InitStructure.SchmittTrigger = DISABLE;                  
    GPIO_InitStructure.Alternate = GPIO0_2_AF_MIO_D2;           // 复用为MIO D2
    LHL_GPIO_Init(pGPIO0, &GPIO_InitStructure);

    /* 2. 使用MIO之前必须初始化MIO Base */
    LHL_MIO_Init();   
    
    /* 3. 配置MIO UART */    
    hMUART.MIO_UART_BaudRate = 19200;                           // 串口位数8/1/0，建议使用较低波特率
    hMUART.MIO_UART_RX = ENABLE;                                // 使能RX接收        
    hMUART.MIO_UART_RX_Pin = MIO_PIN_1;                         // 设定串口接收引脚P0.1
    hMUART.MIO_UART_RX_Timer = MIO_TIMER_0;                     // RX使用MIO定时器0
    hMUART.MIO_UART_RX_Shifter = MIO_SHIFTER_0;                 // RX使用MIO移位寄存器0
    hMUART.MIO_UART_TX = ENABLE;                                // 使能TX发送
    hMUART.MIO_UART_TX_Pin = MIO_PIN_2;                         // 设定串口发送引脚P0.2
    hMUART.MIO_UART_TX_Timer = MIO_TIMER_1;                     // TX使用MIO定时器1
    hMUART.MIO_UART_TX_Shifter = MIO_SHIFTER_1;                 // TX使用MIO移位寄存器1
    LHL_MIO_UARTInit(&hMUART);
    
    /* 4. 可以用Printf，函数已在下方实现 */
    printf("\n\n***** LH32M0G3x MIO UART Demonstration *****\n\n");
    printf("Board will send back received characters.\n");
    
    while(1)
    {
        /* 5. 可以用堵塞方式接收或发送串口数据，如下所示 */    
        if (LHL_MIO_UARTReceive(&hMUART, &buff, 1) != LHL_OK)
        {
            printf("\nTimeout! Waiting for the host to send any characters...\n");
        }
        LHL_MIO_UARTTransmit(&hMUART, &buff, 1);
    }
}

/* Printf函数实现 */
int fputc(int ch, FILE *f)
{
    unsigned char tmp = (unsigned char)ch;
    (void)f;
    LHL_MIO_UARTTransmit(&hMUART, &tmp, 1);
    return ch;
}

/* 配置时钟树:主频4MHz */
void SystemClock_Config(void)
{
    RCC_ClockInitTypeDef Clock_InitStructure;
    Clock_InitStructure.SYSCLKSource = RCC_SYSCLK_SRC_HSI_DIV2; // SYSCLK = 32M/2 = 16MHz
    Clock_InitStructure.AHBCLKDivider = RCC_SYSCLK_DIV4;        // PCLK = SYSCLK/4 = 4MHz
    Clock_InitStructure.APB1CLKDivider = RCC_HCLK_DIV1;         // PCLK1 = HCLK
    Clock_InitStructure.APB2CLKDivider = RCC_HCLK_DIV1;         // PCKL2 = HCLK
    if (LHL_RCC_ClockConfig(&Clock_InitStructure, LHL_NRM_TIMEOUT) != LHL_OK)
    {
        while(1);                                               // Error Handler
    }
}

/*********************************End of File**********************************/
