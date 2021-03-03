/* boot.c -- entry point at reset
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

void Reset_Handler( void) {
    for( ;;) ;
}

/* end of boot.c */
