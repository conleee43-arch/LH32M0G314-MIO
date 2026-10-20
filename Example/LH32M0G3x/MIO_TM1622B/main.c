/******************************************************************************
 * @file    main.c
 * @brief   TM1622B 写功能极简测试（当前硬件仅支持写，不调用任何读接口）
 * @note    测试方法：TM1622B_Init 后 Fill(0x0F) 全亮 / Clear() 全灭，1 秒交替。
 *          判据：LCD 以 1 秒节奏全亮/全灭交替 = 命令帧 + 连续写 + 时序全通；
 *                一直全灭先抓 P1.5(WR) 有无 100kHz 脉冲串分清 MCU 侧/芯片侧。
 *          抓配置时序：CS(P1.7) 下降沿触发、复位后抓，Init 内前两帧为
 *          SYS EN / LCD ON 命令帧（12 位，DATA 线上序应为 1,0,0,C8..C0，
 *          按 WR 上升沿采样），其后是 Clear 的连续写帧。
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "lh32m0xx_lhl.h"
#include "tm1622b_driver.h"

/* Private declaration -------------------------------------------------------*/
void SystemClock_Config(void);

/* Functions -----------------------------------------------------------------*/
int main(void)
{
    /* 上电后IWDG默认开启，若无处理约20秒后复位 */
    LHL_IWDG_Stop();

    /* 1. 配置系统时钟到4MHz——必须在 TM1622B_Init 之前：
          驱动在 Init 时按当时 PCLK2 计算 WR/RD 分频 */
    SystemClock_Config();

    /* 2. 初始化 TM1622B（内部依次发 SYS EN、LCD ON 命令并清屏，
          结束时为全灭状态） */
    TM1622B_Init();

    /* 3. 全亮 / 全灭交替（只写不读） */
    while (1)
    {
        TM1622B_Fill(0x0F);        /* 64 个地址全写 0xF：全亮 */
        LHL_Delay_uS(1000000UL);   /* 1 秒 */
        TM1622B_Clear();           /* 64 个地址全写 0：全灭 */
        LHL_Delay_uS(1000000UL);   /* 1 秒 */
    }
}


/* 使用内部HSI时钟，将HCLK/APB时钟统一配置到4MHz（沿用模板配置，未改动） */
void SystemClock_Config(void)
{
    RCC_ClockInitTypeDef Clock_InitStructure;

    Clock_InitStructure.SYSCLKSource = RCC_SYSCLK_SRC_HSI_DIV2; /* 32MHz HSI / 2 = 16MHz */
    Clock_InitStructure.AHBCLKDivider = RCC_SYSCLK_DIV4;        /* HCLK = 16MHz / 4 = 4MHz */
    Clock_InitStructure.APB1CLKDivider = RCC_HCLK_DIV1;         /* PCLK1 = HCLK = 4MHz */
    Clock_InitStructure.APB2CLKDivider = RCC_HCLK_DIV1;         /* PCLK2 = HCLK = 4MHz */

    if (LHL_RCC_ClockConfig(&Clock_InitStructure, LHL_NRM_TIMEOUT) != LHL_OK)
    {
        while (1)
        {
        }
    }
}

/*********************************End of File**********************************/
