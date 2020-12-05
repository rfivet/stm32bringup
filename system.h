/* system.h -- system services      */
/* Copyright (c) 2020 Renaud Fivet  */

extern volatile unsigned uptime ;   /* seconds elapsed since boot */

int init( void) ;           /* System initialization, called once at startup */

void kputc( unsigned char c) ;      /* character output */
int  kputs( const char s[]) ;       /* string output */
void yield( void) ;                 /* give way */

/* end of system.h */
