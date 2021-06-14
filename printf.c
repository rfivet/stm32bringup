/* printf.c -- format and print data
** Copyright (c) 2020-2021 Renaud Fivet
** v3: %[flags][width]type
** v2: zero padding and uppercase/lowercase for hexadecimal
** v1: type=c,u,x,X,i,d,s,%
*/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "system.h" /* kputc(), kputs() */

size_t strlen( const char *s) {
    const char *end = s - 1 ;
    while( *++end) ;

    return end - s ;
}

#define ZEROPAD     (1 << 3)
#define LEFTALIGN   (3 << 2)
#define LEFTMASK    4
#define PLUSSIGN    (1 << 1)
#define BLANKSIGN   (1 << 0)

static const unsigned char base[] = {
    16, /* Xx 0x58 0x78 */
    10, /* iu 0x69 0x75 */
    2,  /* b  0x62 unused */
    8,  /* o  0x6F */
} ;

static const char signs[] = { 0, ' ', '+', '+'} ;

static int kputpad( char c, int len) {
    int cnt ;

    if( len > 0) {
        cnt = len ;
        while( len--)
            kputc( c) ;
    } else
        cnt = 0 ;

    return cnt ;
}

static int kputu( unsigned u, int padlen, char fmt) {
    char s[ 12] ;                   /* room for 11 octal digit + EOS */
    char *p = &s[ sizeof s - 1] ;   /* point to last byte */
    char signprefix ;

    *p = 0 ;                        /* null terminated string */

    int flags = padlen >> 28 ;
    padlen &= 0x0FFFFFFF ;          /* clear flags */
    if( fmt == 'i' && (0 > (int) u)) {
        u = - (int) u ;
        signprefix = '-' ;
    } else
        signprefix = signs[ flags & 3] ;

    if( signprefix)
        padlen -= 1 ;

    unsigned d = base[ fmt & 3] ;
    fmt = fmt & 0x20 ;              /* set uppercase bit */
    do {
        *--p = "0123456789ABCDEF"[ u % d] | fmt ;
        padlen -= 1 ;
        u /= d ;
    } while( u) ;

/* reuse u to calculate output length */
    flags >>= 2 ;
    if( !flags)     /* Right align */
        u += kputpad( ' ', padlen) ;

    if( signprefix) {
        kputc( signprefix) ;
        u += 1 ;
    }

    if( flags == (ZEROPAD >> 2))
        u += kputpad( '0', padlen) ;

    u += kputs( p) ;
    if( flags == (LEFTALIGN >> 2))
        u += kputpad( ' ', padlen) ;

    return u ;
}

int printf( const char *fmt, ...) {
    va_list ap ;
    int cnt = 0 ;
    int c ; /* current char in format string */

    va_start( ap, fmt) ;
    while( ( c = *fmt++) != 0)
        if( c != '%') {
            cnt += 1 ; kputc( c) ;
        } else {
        /* %[flags][width]type */
            int flags = 0 ;
            int width = 0 ;

        /* flags and width */
            for( ;;) {
                c = *fmt++ ;
                switch( c) {
                case ' ':
                    flags |= BLANKSIGN ;
                    continue ;
                case '+':
                    flags |= PLUSSIGN ;
                    continue ;
                case '-':
                    flags |= LEFTALIGN ;
                    continue ;
                case '0':
                    flags |= ZEROPAD ;
                    continue ;
                case '*':
                    width = va_arg( ap, int) ;
                    c = *fmt++ ;
                    break ;
                default:
                    while( c >= '0' && c <= '9') {
                        width = width * 10 + ( c - '0') ;
                        c = *fmt++ ;
                    }
                }

                break ;
            }

        /* type */
            if( c == 0)
                break ;

            switch( c) {
            case 'c':
                flags >>= 2 ;
                if( !flags) /* right aligned */
                    cnt += kputpad( ' ', width - 1) ;

                cnt += 1 ; kputc( va_arg( ap, int /* char */)) ;
                if( flags == (LEFTALIGN >> 2))
                    cnt += kputpad( ' ', width - 1) ;

                break ;
            case 'd':
                c = 'i' ;
                /* fallthrough */
            case 'i':
            case 'o':
            case 'u':
            case 'x':
            case 'X':
                cnt += kputu( va_arg( ap, unsigned), width | flags << 28, c) ;
                break ;
            case 's': {
                    char *argp = va_arg( ap, char *) ;
                    flags >>= 2 ;
                    if( !flags) /* right aligned */
                        cnt += kputpad( ' ', width - strlen( argp)) ;

                    int size = kputs( argp) ;
                    cnt += size ;
                    if( flags == (LEFTMASK >> 2))
                        cnt += kputpad( ' ', width - size) ;
                }

                break ;
            default:
                cnt += 1 ; kputc( '%') ;
                /* fallthrough */
            case '%':
                cnt += 1 ; kputc( c) ;
            }
        }

    va_end( ap) ;
    return cnt ;
}

/* end of printf.c */
