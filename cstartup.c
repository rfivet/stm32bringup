/* cstartup.c -- data and bss RAM memory initialization
** Copyright (c) 2020 Renaud Fivet
*/

/* Memory locations defined by linker script */
extern long __StackTop ;        /* &__StackTop points after end of stack */
void Reset_Handler( void) ;     /* Entry point for execution */
extern const long __etext[] ;   /* start of initialized data copy in flash */
extern long __data_start__[] ;
extern long __bss_start__[] ;
extern long __bss_end__ ;       /* &__bss_end__ points after end of bss */

/* Interrupt vector table:
 * 1  Stack Pointer reset value
 * 15 System Exceptions
 * NN Device specific Interrupts
 */
typedef void (*isr_p)( void) ;
isr_p const isr_vector[ 2] __attribute__((section(".isr_vector"))) = {
    (isr_p) &__StackTop,
/* System Exceptions */
    Reset_Handler
} ;

extern int main( void) ;

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

    main() ;
    for( ;;) ;
}

/** Test code: main.c *********************************************************/

const char hexa[] = "0123456789abcdef" ;
long first = 1 ;
long i ;

int main( void) {
    static char c = 'a' ;
    char *cp = &c ;

    *cp += i ;
    i += hexa[ 13] - c + first++ ;
    return 0 ;
}

/* end of cstartup.c */
