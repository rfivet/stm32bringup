/* memcpy.c -- copy memory area     */
/* Copyright (c) 2021 Renaud Fivet  */

#include <string.h>

void *memcpy( void *to, const void *from, size_t n) {
    const char *s = from ;
    char *d = to ;
    while( n--)
        *d++ = *s++ ;

    return to ;
}

/* end of memcpy.c */
