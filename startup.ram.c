/* startup.ram.c -- entry point at reset and C startup
** Copyright (c) 2020-2021 Renaud Fivet
** v7: isr vector mapped to RAM to enable in RAM execution
** v6: device specific interrupts mapped
** v5: System Exceptions mapped
** v4: calls to init() and main()
** v3: data and bss RAM memory initialization
** v2: SysTick System Exception mapped
** v1: stack and entry point
*/

#include "system.h" /* init() */
#include "stm32f030xx.h"

/* Memory locations defined by linker script */
void __StackTop( void) ;        /* __StackTop points after end of stack */
void Reset_Handler( void) ;     /* Entry point for execution */
extern const long __etext[] ;   /* start of initialized data copy in flash */
extern long __data_start__[] ;
extern long __bss_start__[] ;
extern long __bss_end__ ;       /* &__bss_end__ points after end of bss */

/* Stubs for System Exception Handler */
void Default_Handler( void) ;
#define dflt_hndlr( fun) void fun##_Handler( void) \
                                __attribute__((weak,alias("Default_Handler")))
dflt_hndlr( NMI) ;
dflt_hndlr( HardFault) ;
dflt_hndlr( SVCall) ;
dflt_hndlr( PendSV) ;
dflt_hndlr( SysTick) ;

dflt_hndlr( WWDG) ;
dflt_hndlr( RTC) ;
dflt_hndlr( FLASH) ;
dflt_hndlr( RCC) ;
dflt_hndlr( EXTI0_1) ;
dflt_hndlr( EXTI2_3) ;
dflt_hndlr( EXTI4_15) ;
dflt_hndlr( DMA_CH1) ;
dflt_hndlr( DMA_CH2_3) ;
dflt_hndlr( DMA_CH4_5) ;
dflt_hndlr( ADC) ;
dflt_hndlr( TIM1_BRK_UP_TRG_COM) ;
dflt_hndlr( TIM1_CC) ;
dflt_hndlr( TIM3) ;
dflt_hndlr( TIM6) ;
dflt_hndlr( TIM14) ;
dflt_hndlr( TIM15) ;
dflt_hndlr( TIM16) ;
dflt_hndlr( TIM17) ;
dflt_hndlr( I2C1) ;
dflt_hndlr( I2C2) ;
dflt_hndlr( SPI1) ;
dflt_hndlr( SPI2) ;
dflt_hndlr( USART1) ;
dflt_hndlr( USART2) ;
dflt_hndlr( USART3_4_5_6) ;
dflt_hndlr( USB) ;

/* Interrupt vector table:
 * 1  Stack Pointer reset value
 * 15 System Exceptions
 * 32 Device specific Interrupts
 */
typedef void (*isr_p)( void) ;
isr_p const isr_vector[ 16 + 32] __attribute__((section(".isr_vector"))) = {
    __StackTop,
/* System Exceptions */
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    0,  0,  0,  0,  0,  0,  0,
    SVCall_Handler,
    0,  0,
    PendSV_Handler,
    SysTick_Handler,
/* STM32F030xx specific Interrupts cf RM0360 */
    WWDG_Handler,
    0,
    RTC_Handler,
    FLASH_Handler,
    RCC_Handler,
    EXTI0_1_Handler,
    EXTI2_3_Handler,
    EXTI4_15_Handler,
    0,
    DMA_CH1_Handler,
    DMA_CH2_3_Handler,
    DMA_CH4_5_Handler,
    ADC_Handler,
    TIM1_BRK_UP_TRG_COM_Handler,
    TIM1_CC_Handler,
    0,
    TIM3_Handler,
    TIM6_Handler,
    0,
    TIM14_Handler,
    TIM15_Handler,
    TIM16_Handler,
    TIM17_Handler,
    I2C1_Handler,
    I2C2_Handler,
    SPI1_Handler,
    SPI2_Handler,
    USART1_Handler,
    USART2_Handler,
    USART3_4_5_6_Handler,
    0,
    USB_Handler
} ;

#if RAMISRV
# define ISRV_SIZE (sizeof isr_vector / sizeof *isr_vector)
isr_p ram_vector[ ISRV_SIZE] __attribute__((section(".ram_vector"))) ;
#endif

int main( void) ;

void Reset_Handler( void) {
    const long  *f ;    /* from, source constant data from FLASH */
    long    *t ;        /* to, destination in RAM */

#if RAMISRV
/* Copy isr vector to beginning of RAM */
    for( unsigned i = 0 ; i < ISRV_SIZE ; i++)
        ram_vector[ i] = isr_vector[ i] ;
#endif

/* Assume:
**  __bss_start__ == __data_end__
**  All sections are 4 bytes aligned
*/
    f = __etext ;
    for( t = __data_start__ ; t < __bss_start__ ; t += 1)
        *t = *f++ ;

    while( t < &__bss_end__)
        *t++ = 0 ;

/* Make sure active isr vector is mapped at 0x0 before enabling interrupts */
    RCC_APB2ENR |= RCC_APB2ENR_SYSCFGEN ;           /* Enable SYSCFG */
#if RAMISRV
    SYSCFG_CFGR1 |= 3 ;                             /* Map RAM at 0x0 */
#else
    SYSCFG_CFGR1 &= ~3 ;                            /* Map FLASH at 0x0 */
#endif

    if( init() == 0)
        main() ;

    for( ;;)
        __asm( "WFI") ; /* Wait for interrupt */
}

void Default_Handler( void) {
    for( ;;) ;
}

/* end of startup.ram.c */
