/* puts.c -- write a string to stdout   */
/* Copyright (c) 2020 Renaud Fivet      */

#include <stdio.h>
#include "system.h" /* kputc(), kputs() */

int puts( const char *s) {
    kputs( s) ;
    kputc( '\n') ;
    return 0 ;
}

/* end of puts.c */
