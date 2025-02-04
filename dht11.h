/* dht11.h -- DHT11 API                  */
/* Copyright (c) 2020-2025 Renaud Fivet  */

typedef enum {
    DHT11_SUCCESS,
    DHT11_FAIL_TOUT,
    DHT11_FAIL_CKSUM
} dht11_retv_t ;


/* 5 .. 95 %RH, -20 .. 60 C */
extern unsigned char dht11_humid ;  /* 5 .. 95 %RH */
extern   signed char dht11_tempc ;  /* -20 .. 60 C */
extern unsigned char dht11_tempf ;  /* .0 .. .9 C */
extern          int  dht11_deciC ;  /* -200 .. 600 */


void dht11_init( void) ;
dht11_retv_t dht11_read( void) ;

/* end of dht11.h */
