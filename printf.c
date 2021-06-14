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

#define LEFTALIGN   (1 << 3)
#define ZEROPAD     (1 << 2)
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

static int kputwidth( char *s, int left_aligned, int width) {
    int cnt = left_aligned ? 0 : kputpad( ' ', width - strlen( s)) ;
    int size = kputs( s) ;
    cnt += size ;
    if( left_aligned)
        cnt += kputpad( ' ', width - size) ;

    return cnt ;
}

static int kputu( unsigned u, int flags, int width, char fmt) {
#define MAXDIGITS   11
    char s[ MAXDIGITS + 1] ;        /* room for 11 octal digit + EOS */
                                    /* octal 37777777777, decimal -2147483647 */
    char *p = &s[ sizeof s - 1] ;   /* point to last byte */
    char signprefix ;

    *p = 0 ;                        /* null terminated string */
    if( fmt != 'i')
    /* ouxX (d has been converted to i) */
        signprefix = 0 ;
    else if( 0 > (int) u) {
        u = - (int) u ;
        signprefix = '-' ;
    } else
        signprefix = signs[ flags & 3] ;

    unsigned d = base[ fmt & 3] ;
    fmt = fmt & 0x20 ;              /* set uppercase bit */
    do {
        *--p = "0123456789ABCDEF"[ u % d] | fmt ;
        u /= d ;
    } while( u) ;

    if( signprefix)
        *--p = signprefix ;

    int size = MAXDIGITS - (p - s) ;    /* string length */
/* reuse u to return output length */
    if( width <= size)
        u = kputs( p) ;
    else if( (flags & 0x0C) == ZEROPAD) {   /* LEFTALIGN precedence over ZEROPAD */
    /* handle zero padding */
        if( signprefix)
            kputc( *p++) ;

        kputpad( '0', width - size) ;
        kputs( p) ;
        u = width ;
    } else
    /* handle left and right alignment */
        u = kputwidth( p, flags >> 3, width) ;

    return u ;
}

int printf( const char *fmt, ...) {
    va_list ap ;
    int cnt = 0 ;
    int c ; /* current char in format string */
    char *sp ;

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
                c = va_arg( ap, int) ;
                if( width > 1) {
                /* handle left and right alignment */
                    sp = (char *) &c ;
                    goto s_width ;
                } else {
                    cnt += 1 ; kputc( c) ;
                }

                break ;
            case 'd':
                c = 'i' ;
                /* fallthrough */
            case 'i':
            case 'o':
            case 'u':
            case 'x':
            case 'X':
                cnt += kputu( va_arg( ap, unsigned), flags, width, c) ;
                break ;
            case 's':
                sp = va_arg( ap, char *) ;
                if( width)
                /* handle left and right alignment */
                s_width:
                    cnt += kputwidth( sp, flags >> 3, width) ;
                else
                    cnt += kputs( sp) ;

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
