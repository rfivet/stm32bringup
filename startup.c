/* startup.c -- entry point at reset and C startup
** Copyright (c) 2020-2021 Renaud Fivet
*/

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

/* Interrupt vector table:
 * 1  Stack Pointer reset value
 * 15 System Exceptions
 * NN Device specific Interrupts
 */
typedef void (*isr_p)( void) ;
isr_p const isr_vector[ 16] __attribute__((section(".isr_vector"))) = {
    __StackTop,
/* System Exceptions */
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    0,  0,  0,  0,  0,  0,  0,
    SVCall_Handler,
    0,  0,
    PendSV_Handler,
    SysTick_Handler
} ;

int init( void) ;
int main( void) ;

void Reset_Handler( void) {
    const long  *f ;    /* from, source constant data from FLASH */
    long    *t ;        /* to, destination in RAM */

/* Assume:
**  __bss_start__ == __data_end__
**  All sections are 4 bytes aligned
*/
    f = __etext ;
    for( t = __data_start__ ; t < __bss_start__ ; t += 1)
        *t = *f++ ;

    while( t < &__bss_end__)
        *t++ = 0 ;

    if( init() == 0)
        main() ;

    for( ;;)
        __asm( "WFI") ; /* Wait for interrupt */
}

void Default_Handler( void) {
    for( ;;) ;
}

/* end of startup.c */
