/******************************************************************************
 * @file    tm1622b_driver.h
 * @brief   LH32M0G314 MCU MIO外设驱动 TM1622B LCD驱动芯片头文件
 * @note    封装: QFN40
 *          引脚映射:
 *            - MIO_D0 (P1.4 / Pin 34): DATA (双向数据)
 *            - MIO_D1 (P1.5 / Pin 35): WR   (写时钟，Timer0输出)
 *            - MIO_D2 (P1.6 / Pin 36): RD   (读时钟，Timer1输出)
 *            - GPIO   (P1.7 / Pin 37): CS   (片选，普通GPIO输出，低有效)
 ******************************************************************************/

#ifndef __TM1622B_DRIVER_H__
#define __TM1622B_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lh32m0xx_lhl.h"
#include "lh32m0g3x_mio.h"
#include "lh32m0g3x_gpio.h"
#include "lh32m0g3x_rcc.h"

/* ============================================================================
   引脚与硬件配置宏定义
   ============================================================================ */
#define TM1622B_CS_PORT         pGPIO1
#define TM1622B_CS_PIN          GPIO_PIN_7

#define TM1622B_DATA_PORT       pGPIO1
#define TM1622B_DATA_PIN        GPIO_PIN_4

#define TM1622B_WR_PORT         pGPIO1
#define TM1622B_WR_PIN          GPIO_PIN_5

#define TM1622B_RD_PORT         pGPIO1
#define TM1622B_RD_PIN          GPIO_PIN_6

/* CS 引脚快速控制宏 */
#define TM1622B_CS_LOW()        LHL_GPIO_ResetPins(TM1622B_CS_PORT, TM1622B_CS_PIN)
#define TM1622B_CS_HIGH()       LHL_GPIO_SetPins(TM1622B_CS_PORT, TM1622B_CS_PIN)

/* ============================================================================
   TM1622B 命令字宏定义 (ID=100 + 9位命令字 C8..C0)
   注意：有效操作码是高 8 位 C8..C1，最低位 C0 是无关位 X（手册建议写 0），
         故命令字 = 操作码 << 1。例如 SYS EN 操作码 0x01 -> 命令字 0x002。
         (旧定义把操作码直接当命令字、少移 1 位，会使 SYS EN 发成 SYS DIS、
          LCD ON 发成 SYS EN，真 LCD ON 永不发出。)
   ============================================================================ */
#define TM1622B_CMD_SYS_DIS     0x000   /* 0000_0000_X 系统振荡器关闭，LCD偏置发生器关闭 */
#define TM1622B_CMD_SYS_EN      0x002   /* 0000_0001_X 打开系统振荡器 */
#define TM1622B_CMD_LCD_OFF     0x004   /* 0000_0010_X 关闭LCD偏置发生器 */
#define TM1622B_CMD_LCD_ON      0x006   /* 0000_0011_X 打开LCD偏置发生器 */
#define TM1622B_CMD_TIMER_DIS   0x008   /* 0000_0100_X 关闭定时器输出 */
#define TM1622B_CMD_WDT_DIS     0x00A   /* 0000_0101_X 关闭看门狗定时器 */
#define TM1622B_CMD_TIMER_EN    0x00C   /* 0000_0110_X 打开定时器输出 */
#define TM1622B_CMD_WDT_EN      0x00E   /* 0000_0111_X 打开看门狗定时器 */
#define TM1622B_CMD_TONE_OFF    0x010   /* 0000_1000_X 关闭声音输出 */

/* 注：TM1622B 硬件固定 1/4 偏压，COM 数量由接线决定（开显示后可驱动 2/4/8 COM），
   数据手册命令表中不存在任何 BIAS/COM 配置命令，故不提供相关宏；
   蜂鸣器亦无独立 "TONE ON" 命令（仅 TONE OFF），开启声音需用数据手册的 TONE 2K / TONE 4K 命令。 */

/* ============================================================================
   API 函数声明
   ============================================================================ */

/**
 * @brief  初始化 MIO 外设及 GPIO 引脚用于驱动 TM1622B
 * @param  None
 * @retval None
 */
void TM1622B_Init(void);

/**
 * @brief  向 TM1622B 发送 9位 控制命令
 * @param  cmd9: 9位命令字（对应手册 C8..C0）
 * @retval None
 */
void TM1622B_WriteCmd(uint16_t cmd9);

/**
 * @brief  向指定 RAM 地址写入 4位 数据 (Nibble)
 * @param  addr: RAM 地址 (0x00 ~ 0x3F)
 * @param  data: 4位数据 (D0~D3)
 * @retval None
 */
void TM1622B_WriteRAM(uint8_t addr, uint8_t data);

/**
 * @brief  连续向 RAM 写入多个 4位 数据 (地址自动递增)
 * @param  start_addr: 起始 RAM 地址 (0x00 ~ 0x3F)
 * @param  pData: 4位数据缓冲区指针
 * @param  count: 写入的数据个数 (Nibble数量)
 * @retval None
 */
void TM1622B_WriteRAM_Continuous(uint8_t start_addr, const uint8_t *pData, uint8_t count);

/**
 * @brief  从指定 RAM 地址读取 4位 数据
 * @param  addr: RAM 地址 (0x00 ~ 0x3F)
 * @retval 4位数据 (D0~D3)
 */
uint8_t TM1622B_ReadRAM(uint8_t addr);

/**
 * @brief  连续读取多个 RAM 地址的 4位 数据
 * @param  start_addr: 起始 RAM 地址 (0x00 ~ 0x3F)
 * @param  pData: 接收缓冲区指针
 * @param  count: 读取的数据个数 (Nibble数量)
 * @retval None
 */
void TM1622B_ReadRAM_Continuous(uint8_t start_addr, uint8_t *pData, uint8_t count);

/**
 * @brief  清空 TM1622B 所有显存 (全灭)
 * @param  None
 * @retval None
 */
void TM1622B_Clear(void);

/**
 * @brief  填充 TM1622B 所有显存 (如 0x0F 表示全亮)
 * @param  pattern: 填充模式 (4位数据)
 * @retval None
 */
void TM1622B_Fill(uint8_t pattern);

/**
 * @brief  开启 LCD 显示及系统振荡器
 * @param  None
 * @retval None
 */
void TM1622B_DisplayOn(void);

/**
 * @brief  关闭 LCD 显示
 * @param  None
 * @retval None
 */
void TM1622B_DisplayOff(void);

#ifdef __cplusplus
}
#endif

#endif /* __TM1622B_DRIVER_H__ */
