/* putchar.c -- write a character to stdout */
/* Copyright (c) 2020-2021 Renaud Fivet     */

#include <stdio.h>
#include "system.h" /* kputc() */

int (putchar)( int c) { /* putchar may be defined as macro in stdio.h */
    kputc( c) ;
    return c ;
}

/* end of putchar.c */
