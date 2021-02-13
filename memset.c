/* memset.c -- fill memory area     */
/* Copyright (c) 2021 Renaud Fivet  */

#include <string.h>

void *memset( void *s, int c, size_t n) {
    char *p = s ;
    while( n--)
        *p++ = c ;

    return s ;
}

/* end of memset.c */
