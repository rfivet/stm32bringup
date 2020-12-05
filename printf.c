/* printf.c -- format and print data    */
/* Copyright (c) 2020 Renaud Fivet      */

#include <stdarg.h>
#include <stdio.h>
#include "system.h" /* kputc(), kputs() */

static int kputu( unsigned u, unsigned d) {
    char s[ 12] ;                   /* room for 11 octal digit + EOS */
    char *p = &s[ sizeof s - 1] ;   /* point to last byte */

    *p = 0 ;                        /* null terminated string */
    do {
        unsigned r = u % d ;
        u /= d ;
        *--p = "0123456789ABCDEF"[ r] ;
    } while( u) ;

    return kputs( p) ;
}

static int kputi( int i) {
    int flag = i < 0 ;
    if( flag) {
        i = -i ;
        kputc( '-') ;
    }

    return flag + kputu( i, 10) ;
}

int printf( const char *fmt, ...) {
    va_list ap ;
    int cnt = 0 ;
    int c ; /* current char in format string */

    va_start( ap, fmt) ;
    while( ( c = *fmt++) != 0)
        if( c != '%') {
            cnt += 1 ; kputc( c) ;
        } else if( ( c = *fmt++) == 0) {
            cnt += 1 ; kputc( '%') ;
            break ;
        } else
            switch( c) {
            case 'c':
                cnt += 1 ; kputc( va_arg( ap, int /* char */)) ;
                break ;
            case 'o':
                cnt += kputu( va_arg( ap, unsigned), 8) ;
                break ;
            case 'u':
                cnt += kputu( va_arg( ap, unsigned), 10) ;
                break ;
            case 'x':
            case 'X':
                cnt += kputu( va_arg( ap, unsigned), 16) ;
                break ;
            case 'i':
            case 'd':
                cnt += kputi( va_arg( ap, int)) ;
                break ;
            case 's':
                cnt += kputs( va_arg( ap, char *)) ;
                break ;
            default:
                cnt += 1 ; kputc( '%') ;
                /* fallthrough */
            case '%':
                cnt += 1 ; kputc( c) ;
            }

    va_end( ap) ;
    return cnt ;
}

/* end of printf.c */
