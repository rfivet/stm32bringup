/* putchar.c -- write a character to stdout */
/* Copyright (c) 2020 Renaud Fivet          */

#include <stdio.h>
#include "system.h" /* kputc() */

int putchar( int c) {
    kputc( c) ;
    return c ;
}

/* end of putchar.c */
