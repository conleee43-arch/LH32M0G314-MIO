;/**************************************************************************//**
; * @file     startup_ARMCM0.s
; * @brief    CMSIS Core Device Startup File for
; *           ARMCM0 Device Series
; * @version  V1.01
; * @date     16. October 2025
; *
; * @note     Converted to IAR Assembly Syntax
; *
; ******************************************************************************/

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)
        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        EXTERN  SystemInit
        PUBLIC  __vector_table

        DATA

; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000400

; <h> Heap Configuration
;   <o> Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000400

; Vector Table Mapped to Address 0 at Reset
__vector_table
        DCD     sfe(CSTACK)                 ; Top of Stack
        DCD     Reset_Handler               ; Reset Handler
        DCD     NMI_Handler                 ; NMI Handler
        DCD     HardFault_Handler           ; Hard Fault Handler
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     SVC_Handler                 ; SVCall Handler
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     PendSV_Handler              ; PendSV Handler
        DCD     SysTick_Handler             ; SysTick Handler

        ; External Interrupts
        DCD     0                           ; Reserved
        DCD     PVD_IRQHandler              ; PVD 1
        DCD     ADC_ERR_IRQHandler          ; ADC ERROR 2
        DCD     RTC_IRQHandler              ; RTC 3
        DCD     CROSSLINK_IRQHandler        ; CROSSLINK 4
        DCD     RCC_IRQHandler              ; RCC 5
        DCD     EXTI0_1_IRQHandler          ; EXTI Line 0,1 6
        DCD     EXTI2_3_IRQHandler          ; EXTI Line 2,3 7
        DCD     EXTI4_7_IRQHandler          ; EXTI Line 4..7 8
        DCD     DMA1_CH0_IRQHandler         ; DMA1 Channel 0 9
        DCD     DMA1_CH1_IRQHandler         ; DMA1 Channel 1 10
        DCD     DMA1_CH2_3_IRQHandler       ; DMA1 Channel 2,3 11
        DCD     SPI0_IRQHandler             ; SPI0 12
        DCD     UART0_IRQHandler            ; UART0 13
        DCD     RTCAlarm_IRQHandler         ; RTCAlarm 14
        DCD     QSPI_IRQHandler             ; QSPI FLASH 15
        DCD     MIO_IRQHandler              ; MIO 16
        DCD     SPI1_IRQHandler             ; SPI1 17
        DCD     ADC0_IRQHandler             ; ADC0 ready 18
        DCD     ADC1_IRQHandler             ; ADC1 ready 19
        DCD     TIM5_IRQHandler             ; TIM5 20
        DCD     UART1_IRQHandler            ; UART1 21
        DCD     TIM6_IRQHandler             ; TIM6 22
        DCD     EEPROM_IRQHandler           ; AFE EEPROM prog 23
        DCD     EXTI10_11_IRQHandler        ; EXTI Line 10,11 24
        DCD     EXTI12_13_IRQHandler        ; EXTI Line 12,13 25
        DCD     LPTIM2_IRQHandler           ; LPTIM2 trigger 26
        DCD     TIM1_IRQHandler             ; TIM1 Global 27
        DCD     TIM2_IRQHandler             ; TIM2 Global 28
        DCD     LPTIM1_IRQHandler           ; LPTIM1 trigger 29
        DCD     MACL_IRQHandler             ; MACL RDY,ERR 30
        DCD     I2C1_EV_IRQHandler          ; I2C1 Event 31

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB

; Reset Handler
        PUBWEAK Reset_Handler
        SECTION .text:CODE:NOROOT:REORDER(2)
Reset_Handler
        LDR     R0, =SystemInit
        BLX     R0
        LDR     R0, =__iar_program_start
        BX      R0

; Dummy Exception Handlers (infinite loops which can be modified)
        PUBWEAK NMI_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
NMI_Handler
        B       NMI_Handler

        PUBWEAK HardFault_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
HardFault_Handler
        B       HardFault_Handler

        PUBWEAK SVC_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
SVC_Handler
        B       SVC_Handler

        PUBWEAK PendSV_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
PendSV_Handler
        B       PendSV_Handler

        PUBWEAK SysTick_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
SysTick_Handler
        B       SysTick_Handler

; Default External Interrupt Handlers
        PUBWEAK PVD_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
PVD_IRQHandler
        B       PVD_IRQHandler

        PUBWEAK ADC_ERR_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
ADC_ERR_IRQHandler
        B       ADC_ERR_IRQHandler

        PUBWEAK RTC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
RTC_IRQHandler
        B       RTC_IRQHandler

        PUBWEAK CROSSLINK_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
CROSSLINK_IRQHandler
        B       CROSSLINK_IRQHandler

        PUBWEAK RCC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
RCC_IRQHandler
        B       RCC_IRQHandler

        PUBWEAK EXTI0_1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI0_1_IRQHandler
        B       EXTI0_1_IRQHandler

        PUBWEAK EXTI2_3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI2_3_IRQHandler
        B       EXTI2_3_IRQHandler

        PUBWEAK EXTI4_7_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI4_7_IRQHandler
        B       EXTI4_7_IRQHandler

        PUBWEAK DMA1_CH0_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_CH0_IRQHandler
        B       DMA1_CH0_IRQHandler

        PUBWEAK DMA1_CH1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_CH1_IRQHandler
        B       DMA1_CH1_IRQHandler

        PUBWEAK DMA1_CH2_3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_CH2_3_IRQHandler
        B       DMA1_CH2_3_IRQHandler

        PUBWEAK SPI0_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SPI0_IRQHandler
        B       SPI0_IRQHandler

        PUBWEAK UART0_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
UART0_IRQHandler
        B       UART0_IRQHandler

        PUBWEAK RTCAlarm_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
RTCAlarm_IRQHandler
        B       RTCAlarm_IRQHandler

        PUBWEAK QSPI_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
QSPI_IRQHandler
        B       QSPI_IRQHandler

        PUBWEAK MIO_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
MIO_IRQHandler
        B       MIO_IRQHandler

        PUBWEAK SPI1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SPI1_IRQHandler
        B       SPI1_IRQHandler

        PUBWEAK ADC0_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
ADC0_IRQHandler
        B       ADC0_IRQHandler

        PUBWEAK ADC1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
ADC1_IRQHandler
        B       ADC1_IRQHandler

        PUBWEAK TIM5_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM5_IRQHandler
        B       TIM5_IRQHandler

        PUBWEAK UART1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
UART1_IRQHandler
        B       UART1_IRQHandler

        PUBWEAK TIM6_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM6_IRQHandler
        B       TIM6_IRQHandler

        PUBWEAK EEPROM_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EEPROM_IRQHandler
        B       EEPROM_IRQHandler

        PUBWEAK EXTI10_11_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI10_11_IRQHandler
        B       EXTI10_11_IRQHandler

        PUBWEAK EXTI12_13_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI12_13_IRQHandler
        B       EXTI12_13_IRQHandler

        PUBWEAK LPTIM2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
LPTIM2_IRQHandler
        B       LPTIM2_IRQHandler

        PUBWEAK TIM1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM1_IRQHandler
        B       TIM1_IRQHandler

        PUBWEAK TIM2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM2_IRQHandler
        B       TIM2_IRQHandler

        PUBWEAK LPTIM1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
LPTIM1_IRQHandler
        B       LPTIM1_IRQHandler

        PUBWEAK MACL_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
MACL_IRQHandler
        B       MACL_IRQHandler

        PUBWEAK I2C1_EV_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C1_EV_IRQHandler
        B       I2C1_EV_IRQHandler

        END