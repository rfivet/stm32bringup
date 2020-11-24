/* ledtick.c -- System Tick driven user LED blink
** Copyright (c) 2020 Renaud Fivet
*/

/* Memory locations defined by linker script */
extern long __StackTop ;        /* &__StackTop points after end of stack */
void Reset_Handler( void) ;     /* Entry point for execution */

void SysTick_Handler( void) ;

/* Interrupt vector table:
 * 1  Stack Pointer reset value
 * 15 System Exceptions
 * NN Device specific Interrupts
 */
typedef void (*isr_p)( void) ;
isr_p const isr_vector[ 16] __attribute__((section(".isr_vector"))) = {
    (isr_p) &__StackTop,
/* System Exceptions */
    Reset_Handler,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    SysTick_Handler
} ;

#define SYSTICK             ((volatile long *) 0xE000E010)
#define SYSTICK_CSR         SYSTICK[ 0]
#define SYSTICK_RVR         SYSTICK[ 1]
#define SYSTICK_CVR         SYSTICK[ 2]

#define RCC                 ((volatile long *) 0x40021000)
#define RCC_AHBENR          RCC[ 5]
#define RCC_AHBENR_IOPBEN   0x00040000  /*  18: I/O port B clock enable */

#define GPIOB               ((volatile long *) 0x48000400)
#define GPIOB_MODER         GPIOB[ 0]
#define GPIOB_ODR           GPIOB[ 5]

void Reset_Handler( void) {
/* By default SYSCLK == HSI [8MHZ] */

/* SYSTICK */
    SYSTICK_RVR = 1000000 - 1 ;     /* HBA / 8 */
    SYSTICK_CVR = 0 ;
    SYSTICK_CSR = 3 ;               /* HBA / 8, Interrupt ON, Enable */
    /* SysTick_Handler will execute every 1s from now on */

/* User LED ON */
    RCC_AHBENR |= RCC_AHBENR_IOPBEN ;   /* Enable IOPB periph */
    GPIOB_MODER |= 1 << (1 * 2) ;       /* PB1 Output [01], over default 00 */
    /* OTYPER Push-Pull by default */
    /* PB1 output default LOW at reset */

    for( ;;)
        __asm( "WFI") ; /* Wait for interrupt */
}

void SysTick_Handler( void) {
    GPIOB_ODR ^= 1 << 1 ;   /* Toggle PB1 (User LED) */
}

/* end of ledtick.c */
