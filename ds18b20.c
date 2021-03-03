/* ds18b20.c -- 1-Wire digital thermometer */
/* Copyright (c) 2020 Renaud Fivet */

#include "ds18b20.h"    /* implements DS18B20 API */

#include "system.h"     /* gpioa_(), usleep() */

#define DIO 13
#define input()     gpioa_input( DIO)
#define output()    gpioa_output( DIO)
#define bread()     gpioa_read( DIO)

#define MAX_RETRIES 999
#define wait_level( lvl) \
    retries = MAX_RETRIES ; \
    while( bread() != lvl) \
        if( retries-- == 0) \
            return DS18B20_FAIL_TOUT

void ds18b20_init( void) {
    input() ;           /* Wire floating, HIGH by pull-up */
}

static ds18b20_retv_t initialization( void) {
/* Reset */
    output() ;          /* Wire LOW */
    usleep( 480) ;
    input() ;           /* Wire floating, HIGH by pull-up */

/* Presence */
    int retries ;
    wait_level( HIGH) ; /* Pull-up LOW -> HIGH, T1 */
    wait_level( LOW) ;  /* DS18B20 asserts line to LOW, T2, T2 - T1 = 15~60us */
    wait_level( HIGH) ; /* DS18B20 releases lines, Pull-up LOW -> HIGH, T3
                        ** T3 - T2 = 60~240us */
    usleep( 405) ;      /* 480 = 405 + 15 + 60 */

    return DS18B20_SUCCESS ;
}

static void write( unsigned char uc) {
/* Transmit byte, least significant bit first */
    for( unsigned char curbit = 1 ; curbit ; curbit <<= 1) {
    /* Transmit a bit takes 60us + 1us between transmit */
    /* Write 1: <15us LOW */
    /* Write 0:  60us LOW */
        unsigned t = uc & curbit ? 13 : 60 ;
        output() ;      /* Wire LOW */
        usleep( t) ;
        input() ;       /* Wire floating, HIGH by pull-up */
        usleep( 61 - t) ;
    }
}

static iolvl_t poll( void) {
    output() ;  /* Wire LOW */
    usleep( 1) ;
    input() ;   /* Wire floating, HIGH by pull-up */
    usleep( 5) ;
    iolvl_t bit = bread() ;
    usleep( 55) ;
    return bit ;
}

static unsigned char read( unsigned char *p, int size) {
    unsigned char crc = 0 ;

    while( size--) {
    /* Receive byte, least significant bit first */
        unsigned char uc = 0 ;
        for( unsigned char curbit = 1 ; curbit ; curbit <<= 1) {
        /* read bit */
            int v = poll() ;
            if( v)
                uc |= curbit ;

        /* update CRC */
            v ^= crc ;
            crc >>= 1 ;
            if( v & 1)
                crc ^= 0x119 >> 1 ; /* reverse POLY = x^8 + x^5 + x^4 + 1 */
        }

    /* store byte */
        *p++ = uc ;
    }

    return crc ;
}

static ds18b20_retv_t read_scratchpad( unsigned char scratchpad[]) {
    ds18b20_retv_t ret = initialization() ;
    if( ret != DS18B20_SUCCESS)
        return ret ;

    write( 0xCC) ;  /* Skip ROM */
    write( 0xBE) ;  /* Read Scratchpad */
    return read( scratchpad, 9) ? DS18B20_FAIL_CRC : DS18B20_SUCCESS ;
}

ds18b20_retv_t ds18b20_convert( void) {
    ds18b20_retv_t ret ;

    ret = initialization() ;
    if( ret != DS18B20_SUCCESS)
        return ret ;

    write( 0xCC) ;  /* Skip ROM */
    write( 0x44) ;  /* Convert T */
    return DS18B20_SUCCESS ;
}

ds18b20_retv_t ds18b20_fetch( short *deciCtemp) { /* -550~1250 = -55.0~125.0 C */
    ds18b20_retv_t ret ;
    unsigned char vals[ 9] ;    /* scratchpad */

    ret = read_scratchpad( vals) ;
    if( ret != DS18B20_SUCCESS)
        return ret ;

    *deciCtemp = *((short *) vals) * 10 / 16 ;
    return DS18B20_SUCCESS ;
}

ds18b20_retv_t ds18b20_read( short *deciCtemp) { /* -550~1250 = -55.0~125.0 C */
    ds18b20_retv_t ret ;

    ret = ds18b20_convert() ;
    if( ret != DS18B20_SUCCESS)
        return ret ;

    do
        usleep( 4000) ;
    while( poll() == LOW) ; /* up to 93.75ms for 9 bits, 750ms for 12 bits */

    return ds18b20_fetch( deciCtemp) ;
}

ds18b20_retv_t ds18b20_resolution( unsigned res) {  /* 9..12 bits  */
    ds18b20_retv_t ret ;
    unsigned char vals[ 9] ;    /* scratchpad */
    unsigned char curres ;

/* read scratchpad */
    ret = read_scratchpad( vals) ;
    if( ret != DS18B20_SUCCESS)
        return ret ;

/* update resolution if current value is different than requested */
    res = (res - 9) & 3 ;
    curres = vals[ 4] >> 5 ;
    if( curres != res) {
        vals[ 4] = (vals[ 4] & 0x1F) | (res << 5) ;
        ret = initialization() ;
        if( ret != DS18B20_SUCCESS)
            return ret ;

        write( 0xCC) ;  /* Skip ROM */
        write( 0x4E) ;  /* Write Scratchpad */
        write( vals[ 2]) ;
        write( vals[ 3]) ;
        write( vals[ 4]) ;
    }

    return DS18B20_SUCCESS ;
}

/* end of ds18b20.c */
