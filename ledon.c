/* ledon.c -- user LED on
** Copyright (c) 2020-2021 Renaud Fivet
*/

/* Memory locations defined by linker script */
void __StackTop( void) ;        /* __StackTop points after end of stack */
void Reset_Handler( void) ;     /* Entry point for execution */

/* Interrupt vector table:
 * 1  Stack Pointer reset value
 * 15 System Exceptions
 * NN Device specific Interrupts
 */
typedef void (*isr_p)( void) ;
isr_p const isr_vector[ 2] __attribute__((section(".isr_vector"))) = {
    __StackTop,
/* System Exceptions */
    Reset_Handler
} ;

#define RCC                 ((volatile long *) 0x40021000)
#define RCC_AHBENR          RCC[ 5]
#define RCC_AHBENR_IOPBEN   0x00040000  /*  18: I/O port B clock enable */

#define GPIOB               ((volatile long *) 0x48000400)
#define GPIOB_MODER         GPIOB[ 0]

void Reset_Handler( void) {
/* User LED ON */
    RCC_AHBENR |= RCC_AHBENR_IOPBEN ;   /* Enable IOPB periph */
    GPIOB_MODER |= 1 << (1 * 2) ;       /* PB1 Output [01], over default 00 */
    /* OTYPER Push-Pull by default */
    /* PB1 output default LOW at reset */
    for( ;;) ;
}

/* end of ledon.c */
