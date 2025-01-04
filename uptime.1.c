/* uptime.1.c -- tells how long the system has been running
** Copyright (c) 2020-2023 Renaud Fivet
**
** v1 displays the number of seconds elapsed since boot
*/

#include <stdio.h>

extern volatile unsigned uptime ;
extern void kputc( unsigned char c) ;

void kputu( unsigned u) {
    unsigned r = u % 10 ;
    u /= 10 ;
    if( u)
        kputu( u) ;

    kputc( '0' + r) ;
}

int main( void) {
    unsigned last = 0 ;

    for( ;;)
        if( last != uptime) {
            last = uptime ;
            kputu( last) ;
            puts( " sec") ;
        } else
            __asm( "WFI") ; /* Wait for System Tick Interrupt */
}

/* end of uptime.1.c */
