/* uptime.c -- tells how long the system has been running   */
/* Copyright (c) 2020-2025 Renaud Fivet                     */

#include <stdio.h>
#include "system.h" /* uptime, yield() */

static void display( unsigned u, const char *s) {
    if( u)
        printf( " %d %s%s", u, s, &"s"[ u <= 1]) ;
}

int main( void) {
    unsigned last = 0 ;

    for( ;;)
        if( last != uptime) {
            unsigned w, d, h, m ,s ;

            last = uptime ;
            d = h = m = 0 ;
            s = last % 60 ;
            w = last / 60 ;
            if( w) {
                m = w % 60 ;
                w /= 60 ;
                if( w) {
                    h = w % 24 ;
                    w /= 24 ;
                    if( w) {
                        d = w % 7 ;
                        w /= 7 ;
                    }
                }
            }

            printf( "up") ;
            display( w, "week") ;
            display( d, "day") ;
            display( h, "hour") ;
            display( m, "minute") ;
            display( s, "second") ;
            printf( "\n") ;
        } else
            yield() ;   /* Wait for System Tick Interrupt */
}

/* end of uptime.c */
