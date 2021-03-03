/* dht11main.c -- sample DHT11 sensor   */
/* Copyright (c) 2020-2021 Renaud Fivet */
#include <stdio.h>

#include "system.h"
#include "dht11.h"

int main( void) {
    static unsigned last ;

    dht11_init() ;
    for( ;;)
        if( last == uptime)
            yield() ;
        else {
            last = uptime ;
            if( 2 == (last % 5))    /* every 5 seconds starting 2s after boot */
                switch( dht11_read()) {
                case DHT11_SUCCESS:
                    printf( "%u%%RH, %d.%uC\n", dht11_humid, dht11_tempc, dht11_tempf) ;
                    break ;
                case DHT11_FAIL_TOUT:
                    puts( "Timeout") ;
                    break ;
                case DHT11_FAIL_CKSUM:
                    puts( "Cksum error") ;
                }
        }
}

/* end of dht11main.c */
