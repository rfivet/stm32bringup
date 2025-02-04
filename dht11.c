/* dht11.c -- DHT11 humidity and temperature sensor reading */
/* Copyright (c) 2020-2025 Renaud Fivet                     */

#include "dht11.h"      /* implements DHT11 API */

#include "system.h"     /* usleep(), gpioa_*() */

#define DIO 13

#define dht11_input()   gpioa_input( DIO)
#define dht11_output()  gpioa_output( DIO)
#define dht11_bread()   gpioa_read( DIO)

#define MAX_RETRIES 200         /* at 48 MHz, 160 retries for 80 us HIGH */
#define is_not_LOW( a) a != LOW
#define is_not_HIGH( a) a == LOW
#define wait_level( lvl) \
    retries = MAX_RETRIES ; \
    while( is_not_##lvl( dht11_bread())) \
        if( retries-- == 0) \
            return DHT11_FAIL_TOUT


/* 5 .. 95 %RH, -20 .. 60 C */
unsigned char dht11_humid ; /* 5 .. 95 %RH */
  signed char dht11_tempc ; /* -20 .. 60 C */
unsigned char dht11_tempf ; /* .0 .. .9 C */
int           dht11_deciC ; /* -200 .. 600 */

void dht11_init( void) {
/* At startup A13 is ALT DIO with Pull Up enabled */
    dht11_input() ;
}

dht11_retv_t dht11_read( void) {
    unsigned char values[ 5] ;

/* Host START: pulls line down for > 18ms then release line, pull-up raises to HIGH */
    dht11_output() ;
    usleep( 18000) ;
    dht11_input() ;

/* DHT START: takes line, 80us low then 80us high */
    int retries ;       /* retry counter */
    wait_level( LOW) ;  /* HIGH -> LOW, starts 80us low */
    wait_level( HIGH) ; /* LOW -> HIGH, ends 80us low, starts 80us high */

/* DHT transmits 40 bits, high bit first
 *  0 coded as 50us low then 26~28us high
 *  1 coded as 50us low then 70us high
 */
    wait_level( LOW) ; /* HIGH -> LOW, ends 80us high, starts 50us low */
    int threshold = (MAX_RETRIES + retries) / 2 ;

    unsigned char sum = 0 ;
    unsigned char v = 0 ;
    for( int idx = 0 ; idx <= 4 ; idx += 1) {
        sum += v ;
        v = 0 ;
        for( unsigned char curbit = 128 ; curbit ; curbit >>= 1) {
        /* Measure duration of HIGH level */
            wait_level( HIGH) ; /* LOW -> HIGH, ends 50us low, starts timed high */
            wait_level( LOW) ;  /* HIGH -> LOW, timed high ends, starts 50us low */
        /* Set bit based on measured HIGH duration */
            if( retries < threshold)    /* 0 == 26~28us, 1 == 70us */
                v |= curbit ;
        }

        values[ idx] = v ;
    }

/* DHT STOP: releases line after 50us, pull-up raises to HIGH */
    wait_level( HIGH) ; /* LOW -> HIGH, ends 50us low, DHT has released the line */

    if( sum != values[ 4])
        return DHT11_FAIL_CKSUM ;

    dht11_humid = values[ 0] ;
    dht11_tempc = values[ 2] ;
    dht11_tempf = values[ 3] ;
    if( dht11_tempf & 0x80) {
        dht11_tempc = -( dht11_tempc + 1) ;
        dht11_tempf ^= 0x80 ;
    }
    
    dht11_deciC = dht11_tempc * 10 + dht11_tempf ;

    return DHT11_SUCCESS ;
}

/* end of file dht11.c */
