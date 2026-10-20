/******************************************************************************
 * @file    tm1622b_driver.c
 * @brief   LH32M0G314 MCU MIO外设驱动 TM1622B LCD驱动芯片源码
 * @details 基于 MIO 硬件定时器与移位器实现非标 4线 串行接口（CS/WR/RD/DATA）
 *          - WR 时钟 (100kHz) 由 Timer0 生成
 *          - RD 时钟 (50kHz)  由 Timer1 生成
 *          - DATA 引脚 (MIO_D0) 由 Shifter0(TX) 和 Shifter1(RX) 分时复用
 ******************************************************************************/

#include "tm1622b_driver.h"

/* ============================================================================
   内部宏定义与硬件寄存器快速操作
   ============================================================================ */

/* MIO 时钟与波特率计算参数
   - MIO_CLK = PCLK2（MIODIV 保持复位默认值 0，即不分频）
   - TIMCMP[7:0] = (MIO_CLK / (2 * Baud)) - 1
   - 目标: WR 100kHz、RD 50kHz（均低于 TM1622B 上限）
   分频值在运行时依据实际 PCLK2 计算（见 tm1622b_update_baud_div）。
   注意: TIMCMP[7:0] 仅 8 位，PCLK2 过高时按 0xFF 封顶——只会降速，不会超频。 */
#define TM1622B_WR_BAUD_HZ      100000U /* WR 目标频率 */
#define TM1622B_RD_BAUD_HZ      50000U  /* RD 目标频率 */

static uint32_t s_mio_wr_baud_div = 39U; /* WR 分频，运行时更新 */
static uint32_t s_mio_rd_baud_div = 79U; /* RD 分频，运行时更新 */

/* 依据实际 MIO 时钟(=PCLK2)计算 WR/RD 的 TIMCMP 分频值 */
static void tm1622b_update_baud_div(void)
{
    uint32_t mio_clk = LHL_RCC_GetPCLK2Freq(); /* MIO_CLK = PCLK2 */

    s_mio_wr_baud_div = (mio_clk / (2U * TM1622B_WR_BAUD_HZ)) - 1U;
    s_mio_rd_baud_div = (mio_clk / (2U * TM1622B_RD_BAUD_HZ)) - 1U;

    if (s_mio_wr_baud_div > 0xFFU) { s_mio_wr_baud_div = 0xFFU; }
    if (s_mio_rd_baud_div > 0xFFU) { s_mio_rd_baud_div = 0xFFU; }
}

/* 帧组合算法辅助宏 */
/* 命令帧: 100(3位ID) + C8..C0(9位命令)，共12位 */
static inline uint32_t tm1622b_frame_cmd(uint16_t cmd9)
{
    return (((uint32_t)(cmd9 & 0x01FF) << 3) | 0x04U); /* 0b100 = 0x04 */
}

/* 写数据帧: 101(3位ID) + A5..A0(6位地址) + D0..D3(4位数据)，共13位 */
static inline uint32_t tm1622b_frame_wr(uint8_t addr, uint8_t nibble)
{
    return (((uint32_t)(nibble & 0x0F) << 9) | ((uint32_t)(addr & 0x3F) << 3) | 0x05U); /* 0b101 = 0x05 */
}

/* 读请求帧: 110(3位ID) + A5..A0(6位地址)，共9位 */
static inline uint32_t tm1622b_frame_rd_req(uint8_t addr)
{
    return (((uint32_t)(addr & 0x3F) << 3) | 0x06U); /* 0b110 = 0x06 */
}

/* 读数据解析: 取 bit[4:1] (丢弃首位装载脉冲) */
static inline uint8_t tm1622b_parse_rd_first(uint32_t raw)
{
    return (uint8_t)((raw >> 1) & 0x0F);
}

/* 连续读后续数据解析: 直接取 bit[3:0] */
static inline uint8_t tm1622b_parse_rd_next(uint32_t raw)
{
    return (uint8_t)(raw & 0x0F);
}

/* 简单的微秒级延时函数 */
static void tm1622b_delay_us(uint32_t us)
{
    volatile uint32_t count = us * 8; /* 对应32MHz主频简易循环 */
    while (count--)
    {
        __NOP();
    }
}

/* ============================================================================
   驱动核心API实现
   ============================================================================ */

/**
 * @brief  初始化 MIO 外设及 GPIO 引脚用于驱动 TM1622B
 */
void TM1622B_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 1. 使能 MIO 外设（GPIO 时钟由 LHL_GPIO_Init 内部自动使能），
       并依实际 PCLK2 计算 WR/RD 分频（MIO_CLK = PCLK2，MIODIV 保持默认不分频） */
    LHL_MIO_Init();
    tm1622b_update_baud_div();

    /* 2. 配置 CS 引脚 (P1.7) 为 GPIO 推挽输出，空闲拉高 */
    GPIO_InitStruct.Pin = TM1622B_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Current = GPIO_CURRENT_8mA;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    LHL_GPIO_Init(TM1622B_CS_PORT, &GPIO_InitStruct);
    TM1622B_CS_HIGH();

    /* 3. 配置 MIO 复用引脚 (P1.4 -> MIO_D0, P1.5 -> MIO_D1, P1.6 -> MIO_D2) */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Current = GPIO_CURRENT_8mA;
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    GPIO_InitStruct.Pin = TM1622B_DATA_PIN; /* P1.4 - MIO_D0 */
    GPIO_InitStruct.Alternate = GPIO1_4_AF_MIO_D0;
    LHL_GPIO_Init(TM1622B_DATA_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = TM1622B_WR_PIN;   /* P1.5 - MIO_D1 */
    GPIO_InitStruct.Alternate = GPIO1_5_AF_MIO_D1;
    LHL_GPIO_Init(TM1622B_WR_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = TM1622B_RD_PIN;   /* P1.6 - MIO_D2 */
    GPIO_InitStruct.Alternate = GPIO1_6_AF_MIO_D2;
    LHL_GPIO_Init(TM1622B_RD_PORT, &GPIO_InitStruct);

    /* 4. MIO 寄存器全局使能 */
    MIO->CTRL = MIO_CTRL_MIOEN_MASK;

    /* 5. 配置 Shifter 0 (TX 发送模式)
       - SMOD = 010 (Transmit)
       - TIMSEL = 0 (Timer0 控制)
       - PINSEL = 0 (MIO_D0)
       - PINCFG = 11 (Output Enabled)
       - TIMPOL = 1 (Falling edge shift)
    */
    MIO->SHIFTCTL[0] = (2U << MIO_SHIFTCTL_SMOD_SHIFT)   |
                       (0U << MIO_SHIFTCTL_TIMSEL_SHIFT) |
                       (0U << MIO_SHIFTCTL_PINSEL_SHIFT) |
                       (3U << MIO_SHIFTCTL_PINCFG_SHIFT) |
                       (1U << MIO_SHIFTCTL_TIMPOL_SHIFT);
    MIO->SHIFTCFG[0] = 0x00000000U;

    /* 6. 配置 Shifter 1 (RX 接收模式)
       - SMOD = 001 (Receive)
       - TIMSEL = 1 (Timer1 控制)
       - PINSEL = 0 (MIO_D0)
       - PINCFG = 00 (Input / Output Disabled)
       - TIMPOL = 1 (Falling edge sample)
    */
    MIO->SHIFTCTL[1] = (1U << MIO_SHIFTCTL_SMOD_SHIFT)   |
                       (1U << MIO_SHIFTCTL_TIMSEL_SHIFT) |
                       (0U << MIO_SHIFTCTL_PINSEL_SHIFT) |
                       (0U << MIO_SHIFTCTL_PINCFG_SHIFT) |
                       (1U << MIO_SHIFTCTL_TIMPOL_SHIFT);
    MIO->SHIFTCFG[1] = 0x00000000U;

    /* 7. 配置 Timer 0 (WR 写时钟)
       - TIMOD = 01 (Dual 8-bit baud/bit mode)
       - PINSEL = 1 (MIO_D1 / Pin 35)
       - PINCFG = 11 (Push-pull output)
       - TRGSRC = 1 (Internal trigger)
       - TRGPOL = 1 (Inverted trigger)
       - TRGSEL = 1 (!SSF0: Trigger when SSF0 clear)
    */
    MIO->TIMCTL[0] = (1U << MIO_TIMCTL_TIMOD_SHIFT)  |
                      (1U << MIO_TIMCTL_PINSEL_SHIFT) |
                      (3U << MIO_TIMCTL_PINCFG_SHIFT) |
                      (1U << MIO_TIMCTL_TRGSRC_SHIFT) |
                      (1U << MIO_TIMCTL_TRGPOL_SHIFT) |
                      (1U << MIO_TIMCTL_TRGSEL_SHIFT);
    MIO->TIMCFG[0] = 0x01002222U;

    /* 8. 配置 Timer 1 (RD 读时钟)
       - TIMOD = 01 (Dual 8-bit baud/bit mode)
       - PINSEL = 2 (MIO_D2 / Pin 36)
       - PINCFG = 11 (Push-pull output)
       - TRGSRC = 1 (Internal trigger)
       - TRGPOL = 1 (Inverted trigger)
       - TRGSEL = 5 (!SSF1: Trigger when SSF1 clear)
    */
    MIO->TIMCTL[1] = (1U << MIO_TIMCTL_TIMOD_SHIFT)  |
                      (2U << MIO_TIMCTL_PINSEL_SHIFT) |
                      (3U << MIO_TIMCTL_PINCFG_SHIFT) |
                      (1U << MIO_TIMCTL_TRGSRC_SHIFT) |
                      (1U << MIO_TIMCTL_TRGPOL_SHIFT) |
                      (5U << MIO_TIMCTL_TRGSEL_SHIFT);
    MIO->TIMCFG[1] = 0x01002222U;

    /* 9. 初始化硬件之后进行 TM1622B 上电命令配置 */
    tm1622b_delay_us(100);
    TM1622B_DisplayOn();
    TM1622B_Clear();
}

/**
 * @brief  发送 9位 命令到 TM1622B
 */
void TM1622B_WriteCmd(uint16_t cmd9)
{
    uint32_t frame = tm1622b_frame_cmd(cmd9);

    /* 拉低 CS 启动写时序 */
    TM1622B_CS_LOW();
    tm1622b_delay_us(1);

    /* 确认 Shifter 0 处于 TX 输出模式 */
    MIO->SHIFTCTL[0] = (2U << MIO_SHIFTCTL_SMOD_SHIFT)   |
                       (0U << MIO_SHIFTCTL_TIMSEL_SHIFT) |
                       (0U << MIO_SHIFTCTL_PINSEL_SHIFT) |
                       (3U << MIO_SHIFTCTL_PINCFG_SHIFT) |
                       (1U << MIO_SHIFTCTL_TIMPOL_SHIFT);

    /* 设置 Timer 0 位数: 12位 (12 * 2 - 1 = 23) */
    MIO->TIMCMP[0] = (23U << 8) | s_mio_wr_baud_div;

    /* 写入 SHIFTBUF0 自动触发发送 */
    MIO->SHIFTBUF[0] = frame;

    /* 等待发送完成 (SSF0 置位) */
    while (!(MIO->SHIFTSTAT & (1U << 0)))
    {
    }

    tm1622b_delay_us(1);
    TM1622B_CS_HIGH();
}

/**
 * @brief  向单个 RAM 地址写入 4位 数据
 */
void TM1622B_WriteRAM(uint8_t addr, uint8_t data)
{
    uint32_t frame = tm1622b_frame_wr(addr, data);

    TM1622B_CS_LOW();
    tm1622b_delay_us(1);

    /* 确保 Shifter 0 处于 TX 输出模式 */
    MIO->SHIFTCTL[0] = (2U << MIO_SHIFTCTL_SMOD_SHIFT)   |
                       (0U << MIO_SHIFTCTL_TIMSEL_SHIFT) |
                       (0U << MIO_SHIFTCTL_PINSEL_SHIFT) |
                       (3U << MIO_SHIFTCTL_PINCFG_SHIFT) |
                       (1U << MIO_SHIFTCTL_TIMPOL_SHIFT);

    /* 设置 Timer 0 位数: 13位 (13 * 2 - 1 = 25) */
    MIO->TIMCMP[0] = (25U << 8) | s_mio_wr_baud_div;

    /* 写入 SHIFTBUF0 启动硬件发送 */
    MIO->SHIFTBUF[0] = frame;

    /* 等待发送完成 */
    while (!(MIO->SHIFTSTAT & (1U << 0)))
    {
    }

    tm1622b_delay_us(1);
    TM1622B_CS_HIGH();
}

/**
 * @brief  连续写入多个 4位 数据 (连续地址)
 */
void TM1622B_WriteRAM_Continuous(uint8_t start_addr, const uint8_t *pData, uint8_t count)
{
    uint8_t curr_addr = start_addr;
    uint8_t remaining = count;
    uint8_t idx = 0;

    if (pData == NULL || count == 0) return;

    TM1622B_CS_LOW();
    tm1622b_delay_us(1);

    /* 确保 Shifter 0 处于 TX 输出模式 */
    MIO->SHIFTCTL[0] = (2U << MIO_SHIFTCTL_SMOD_SHIFT)   |
                       (0U << MIO_SHIFTCTL_TIMSEL_SHIFT) |
                       (0U << MIO_SHIFTCTL_PINSEL_SHIFT) |
                       (3U << MIO_SHIFTCTL_PINCFG_SHIFT) |
                       (1U << MIO_SHIFTCTL_TIMPOL_SHIFT);

    /* 分帧发送 (每帧最多 5 个 nibble，避免超过 32位 缓冲区) */
    while (remaining > 0)
    {
        uint8_t chunk = (remaining > 5) ? 5 : remaining;
        uint32_t frame = 0x05U | ((uint32_t)(curr_addr & 0x3F) << 3); /* 101 + Addr */
        uint8_t bits = 9 + chunk * 4;

        for (uint8_t i = 0; i < chunk; i++)
        {
            frame |= ((uint32_t)(pData[idx + i] & 0x0F) << (9 + i * 4));
        }

        /* 配置位数并触发硬件发送 */
        MIO->TIMCMP[0] = ((bits * 2 - 1) << 8) | s_mio_wr_baud_div;
        MIO->SHIFTBUF[0] = frame;

        while (!(MIO->SHIFTSTAT & (1U << 0)))
        {
        }

        idx += chunk;
        curr_addr += chunk;
        remaining -= chunk;
    }

    tm1622b_delay_us(1);
    TM1622B_CS_HIGH();
}

/**
 * @brief  从指定 RAM 地址读取 4位 数据
 */
uint8_t TM1622B_ReadRAM(uint8_t addr)
{
    uint32_t req_frame = tm1622b_frame_rd_req(addr);
    uint32_t raw_rd = 0;

    TM1622B_CS_LOW();
    tm1622b_delay_us(1);

    /* 1. 发送 9位 读请求帧 (110 + A5..A0) */
    MIO->SHIFTCTL[0] = (2U << MIO_SHIFTCTL_SMOD_SHIFT)   |
                       (0U << MIO_SHIFTCTL_TIMSEL_SHIFT) |
                       (0U << MIO_SHIFTCTL_PINSEL_SHIFT) |
                       (3U << MIO_SHIFTCTL_PINCFG_SHIFT) |
                       (1U << MIO_SHIFTCTL_TIMPOL_SHIFT);

    MIO->TIMCMP[0] = (17U << 8) | s_mio_wr_baud_div; /* 9位: 9*2-1=17 */
    MIO->SHIFTBUF[0] = req_frame;

    while (!(MIO->SHIFTSTAT & (1U << 0)))
    {
    }

    /* 2. 释放 DATA 引脚 (将 MIO_D0 显式设为输入，避免驱动冲突) */
    MIO->SHIFTCTL[0] = (0U << MIO_SHIFTCTL_SMOD_SHIFT)   |
                       (0U << MIO_SHIFTCTL_PINCFG_SHIFT);

    /* 3. 产生 RD 读时钟并接收 5 位 (1位装载脉冲 + 4位数据) */
    MIO->TIMCMP[1] = (9U << 8) | s_mio_rd_baud_div; /* 5位: 5*2-1=9 */
    MIO->SHIFTBUF[1] = 0x00000000U;               /* 触发 Timer1 */

    while (!(MIO->SHIFTSTAT & (1U << 1)))
    {
    }

    /* 读取位反转寄存器，提取位 D0~D3 */
    raw_rd = MIO->SHIFTBUFBIS[1];

    /* 4. 恢复 Shifter 0 的 TX 配置 */
    MIO->SHIFTCTL[0] = (2U << MIO_SHIFTCTL_SMOD_SHIFT)   |
                       (0U << MIO_SHIFTCTL_TIMSEL_SHIFT) |
                       (0U << MIO_SHIFTCTL_PINSEL_SHIFT) |
                       (3U << MIO_SHIFTCTL_PINCFG_SHIFT) |
                       (1U << MIO_SHIFTCTL_TIMPOL_SHIFT);

    tm1622b_delay_us(1);
    TM1622B_CS_HIGH();

    return tm1622b_parse_rd_first(raw_rd);
}

/**
 * @brief  连续读取多个 RAM 地址的 4位 数据
 */
void TM1622B_ReadRAM_Continuous(uint8_t start_addr, uint8_t *pData, uint8_t count)
{
    uint32_t req_frame = tm1622b_frame_rd_req(start_addr);

    if (pData == NULL || count == 0) return;

    TM1622B_CS_LOW();
    tm1622b_delay_us(1);

    /* 1. 发送 9位 读请求帧 */
    MIO->SHIFTCTL[0] = (2U << MIO_SHIFTCTL_SMOD_SHIFT)   |
                       (0U << MIO_SHIFTCTL_TIMSEL_SHIFT) |
                       (0U << MIO_SHIFTCTL_PINSEL_SHIFT) |
                       (3U << MIO_SHIFTCTL_PINCFG_SHIFT) |
                       (1U << MIO_SHIFTCTL_TIMPOL_SHIFT);

    MIO->TIMCMP[0] = (17U << 8) | s_mio_wr_baud_div;
    MIO->SHIFTBUF[0] = req_frame;

    while (!(MIO->SHIFTSTAT & (1U << 0)))
    {
    }

    /* 2. 释放 DATA 引脚 */
    MIO->SHIFTCTL[0] = (0U << MIO_SHIFTCTL_SMOD_SHIFT)   |
                       (0U << MIO_SHIFTCTL_PINCFG_SHIFT);

    /* 3. 连续接收 */
    for (uint8_t i = 0; i < count; i++)
    {
        uint32_t raw_rd = 0;

        if (i == 0)
        {
            /* 首个 nibble 包含 1 位首脉冲装载: 5 位 (TIMCMP = 9) */
            MIO->TIMCMP[1] = (9U << 8) | s_mio_rd_baud_div;
            MIO->SHIFTBUF[1] = 0x00000000U;

            while (!(MIO->SHIFTSTAT & (1U << 1)))
            {
            }

            raw_rd = MIO->SHIFTBUFBIS[1];
            pData[i] = tm1622b_parse_rd_first(raw_rd);
        }
        else
        {
            /* 续读 nibble: 直接读 4 位 (TIMCMP = 7) */
            MIO->TIMCMP[1] = (7U << 8) | s_mio_rd_baud_div;
            MIO->SHIFTBUF[1] = 0x00000000U;

            while (!(MIO->SHIFTSTAT & (1U << 1)))
            {
            }

            raw_rd = MIO->SHIFTBUFBIS[1];
            pData[i] = tm1622b_parse_rd_next(raw_rd);
        }
    }

    /* 4. 恢复 TX 配置 */
    MIO->SHIFTCTL[0] = (2U << MIO_SHIFTCTL_SMOD_SHIFT)   |
                       (0U << MIO_SHIFTCTL_TIMSEL_SHIFT) |
                       (0U << MIO_SHIFTCTL_PINSEL_SHIFT) |
                       (3U << MIO_SHIFTCTL_PINCFG_SHIFT) |
                       (1U << MIO_SHIFTCTL_TIMPOL_SHIFT);

    tm1622b_delay_us(1);
    TM1622B_CS_HIGH();
}

/**
 * @brief  清空 TM1622B 所有显存 (64个地址全写0)
 */
void TM1622B_Clear(void)
{
    uint8_t zero_buf[64] = {0};
    TM1622B_WriteRAM_Continuous(0x00, zero_buf, 64);
}

/**
 * @brief  填充 TM1622B 所有显存
 */
void TM1622B_Fill(uint8_t pattern)
{
    uint8_t pat_buf[64];
    for (int i = 0; i < 64; i++)
    {
        pat_buf[i] = pattern & 0x0F;
    }
    TM1622B_WriteRAM_Continuous(0x00, pat_buf, 64);
}

/**
 * @brief  开启 LCD 显示及振荡器
 */
void TM1622B_DisplayOn(void)
{
    /* TM1622B 固定 1/4 偏压、COM 数量由接线决定，不存在 BIAS/COM 配置命令 */
    TM1622B_WriteCmd(TM1622B_CMD_SYS_EN);
    TM1622B_WriteCmd(TM1622B_CMD_LCD_ON);
}

/**
 * @brief  关闭 LCD 显示
 */
void TM1622B_DisplayOff(void)
{
    TM1622B_WriteCmd(TM1622B_CMD_LCD_OFF);
    TM1622B_WriteCmd(TM1622B_CMD_SYS_DIS);
}
