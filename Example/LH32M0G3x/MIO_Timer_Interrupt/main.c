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
void SystemClock_Config(void);
void UART_Config(void);

/* Functions -----------------------------------------------------------------*/
int main(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;                    
    MIO_TIM_InitTypedef MTIM_InitStructure;

    /* 上电后IWDG默认开启，若无处理约20秒后复位 */
	LHL_IWDG_Stop();
    /* 配置时钟树：主频1MHz */
    SystemClock_Config();
    /* 配置串口0：位数8/1/0，波特率115200bps */        
    UART_Config();   
    printf("\n\n***** LH32M0G3x MIO TIMER Interrupt Demonstration *****\n\n");
    
    /* 1. 初始化MIO和中断用到的GPIO */
    GPIO_InitStructure.Pin = GPIO_PIN_0;                        // P0.0->MIO_D0
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;           
    GPIO_InitStructure.Current = GPIO_CURRENT_4mA;
    GPIO_InitStructure.Pull = GPIO_NOPULL;        
    GPIO_InitStructure.SchmittTrigger = DISABLE;                  
    GPIO_InitStructure.Alternate = GPIO0_0_AF_MIO_D0;
    LHL_GPIO_Init(pGPIO0, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = GPIO_PIN_3;                        // P0.3->MIO_D3
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;          
    GPIO_InitStructure.Pull = GPIO_NOPULL;              
    GPIO_InitStructure.SchmittTrigger = ENABLE;         
    GPIO_InitStructure.Alternate = GPIO0_3_AF_MIO_D3;       
	LHL_GPIO_Init(pGPIO0, &GPIO_InitStructure); 
    
    GPIO_InitStructure.Pin = GPIO_PIN_4;                        // P0.4->普通输出
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;           
    GPIO_InitStructure.Current = GPIO_CURRENT_4mA;
    GPIO_InitStructure.Pull = GPIO_NOPULL;        
    GPIO_InitStructure.SchmittTrigger = DISABLE;                  
    GPIO_InitStructure.Alternate = GPIOx_AF_GPIO;         
    LHL_GPIO_Init(pGPIO0, &GPIO_InitStructure); 

    /* 2. 初始化MIO Base */
    LHL_MIO_Init();   
    
    /* 3. 配置MIO Timer */
    MTIM_InitStructure.Trigger_Select = MIO_TIM_TRIG_PIN(3);        // 触发 MIO_D3
    MTIM_InitStructure.Trigger_Polarity = MIO_TIM_TRIG_ACTIVE_LOW;  // 低电平有效
    MTIM_InitStructure.Trigger_Source = MIO_TIM_TRIG_SRC_INTERNAL;  // XLINK是外部触发源；引脚、移位寄存器和定时器是内部触发源
    MTIM_InitStructure.Pin_Config = MIO_PIN_OUTPUT_ENABLED;         // 输出使能
    MTIM_InitStructure.Pin_Select = MIO_PIN_0;                      // 引脚 MIO_D0
    MTIM_InitStructure.Pin_Polarity = MIO_PIN_ACTIVE_HIGH;          // 高电平有效
    MTIM_InitStructure.Timer_Mode = MIO_TIM_SINGLE_16BIT;           // 单16位模式
    MTIM_InitStructure.Timer_Output = MIO_TIM_OUTPUT_HIGH;                         
    MTIM_InitStructure.Timer_Decrement_Clock = MIO_TIM_MIO_CLOCK;   // 系统时钟1M
    MTIM_InitStructure.Timer_Reset_Condition = MIO_TIM_RST_NEVER;   // 不复位
    MTIM_InitStructure.Timer_Disable_Condition = MIO_TIM_DIS_COMPARE;   // 比较事件时禁用
    MTIM_InitStructure.Timer_Enable_Condition = MIO_TIM_ENA_TRIGGER_RISING_EDGE;    // 触发源上升沿使能
    MTIM_InitStructure.Timer_StopBit_Condition = MIO_TIM_STOP_BIT_DISABLED;         
    MTIM_InitStructure.Timer_StartBit_Condition = MIO_TIM_START_BIT_DISABLED;
    MTIM_InitStructure.Timer_Compare = 60000;                       // 比较值，1M时钟下大约60mS
    
    LHL_MIO_TimerInit(MIO_TIMER_0,&MTIM_InitStructure);
    
    /* 4. 开启MIO Timer中断 */
    LHL_MIO_ITConfig(MIO_TIMER0_IT_TSIE, ENABLE);
    NVIC_EnableIRQ(MIO_IRQn);
    
    while(1)
    {   
        /* 5. 按下按键K1（MIO_D3触发源），P0.0指示MIO Pin的状态，P0.4指示MIO中断的状态 */
    }
}

/* 配置时钟树:主频1MHz */
void SystemClock_Config(void)
{
    RCC_ClockInitTypeDef Clock_InitStructure;
    Clock_InitStructure.SYSCLKSource = RCC_SYSCLK_SRC_HSI_DIV2; // SYSCLK = 32M/2 = 16MHz
    Clock_InitStructure.AHBCLKDivider = RCC_SYSCLK_DIV16;       // PCLK = SYSCLK/16 = 1MHz
    Clock_InitStructure.APB1CLKDivider = RCC_HCLK_DIV1;         // PCLK1 = HCLK
    Clock_InitStructure.APB2CLKDivider = RCC_HCLK_DIV1;         // PCKL2 = HCLK
    if (LHL_RCC_ClockConfig(&Clock_InitStructure, LHL_NRM_TIMEOUT) != LHL_OK)
    {
        while(1);                                               // Error Handler
    }
}

/* 配置串口0：位数8/1/0，波特率9600bps */
void UART_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	UART_InitTypeDef UART_InitStructure;
    
    /* 设置 UART GPIO复用端口 */    
    GPIO_InitStructure.Pin = GPIO_PIN_1;               
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Current = GPIO_CURRENT_4mA;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.SchmittTrigger = ENABLE;
    GPIO_InitStructure.Alternate = GPIO0_1_AF_SIN0;             // AFIO设置
    LHL_GPIO_Init(pGPIO0, &GPIO_InitStructure);                 // P0.1 -> SIN0(UART_RX)
    
    GPIO_InitStructure.Pin = GPIO_PIN_2;               
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Current = GPIO_CURRENT_4mA;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.SchmittTrigger = DISABLE;
    GPIO_InitStructure.Alternate = GPIO0_2_AF_SOUT0;            // AFIO设置
    LHL_GPIO_Init(pGPIO0, &GPIO_InitStructure);                 // P0.2 -> SOUT0(UART_TX)

    /* 设置UART参数 */
    UART_InitStructure.UART_BaudRate = 9600;                    // 最高波特率需小于时钟频率/16
	UART_InitStructure.UART_WordLength = UART_WordLength_8b;    // 数字位数：8
	UART_InitStructure.UART_StopBits = UART_StopBits_1;         // 停止位数：1
	UART_InitStructure.UART_Parity = UART_Parity_None;          // 校验位：无
 	LHL_UART_Init(pUART0, &UART_InitStructure);
}

/*********************************End of File**********************************/
