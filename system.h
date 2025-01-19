/* system.h -- system services      */
/* Copyright (c) 2020 Renaud Fivet  */

extern volatile unsigned uptime ;   /* seconds elapsed since boot */

int init( void) ;           /* System initialization, called once at startup */

void kputc( unsigned char c) ;      /* character output */
int  kputs( const char s[]) ;       /* string output */
void yield( void) ;                 /* give way */

/* GPIOA low level API ********************************************************/

typedef enum {
    LOW = 0,
    HIGH
} iolvl_t ;

void gpioa_input( int pin) ;        /* Configure GPIOA pin as input */
void gpioa_output( int pin) ;       /* Configure GPIOA pin as output */
iolvl_t  gpioa_read( int pin) ;     /* Read level of GPIOA pin */

void usleep( unsigned usecs) ;      /* wait at least usecs us */

typedef enum {
    VNT_INIT,
    VNT_CAL,
    VNT_RAW,
    VNT_VNC
} vnt_cmd_t ;

void adc_vnt( vnt_cmd_t cmd, short *ptrV, short *ptrC) ;
const unsigned short *adc_init( unsigned channels) ;
unsigned adc_convert( void) ;

/* end of system.h */
