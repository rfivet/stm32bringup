/* ds18b20main.c -- sample temperature using 1-Wire temperature sensor */
/* Copyright (c) 2020 Renaud Fivet */

#include <stdio.h>

#include "system.h"     /* uptime */
#include "ds18b20.h"    /* ds18b20_() */

int main( void) {
    unsigned last = 0 ;

    ds18b20_init() ;
    ds18b20_resolution( 12) ;   /* Set highest resolution: 12 bits */
    ds18b20_convert() ;         /* start temperature conversion */
    for( ;;)
        if( last == uptime)
            yield() ;
        else {
            short val ;

            last = uptime ;
            switch( ds18b20_fetch( &val)) {
            case DS18B20_SUCCESS:
                printf( "%i.%i\n", val / 10, val % 10) ;
                break ;
            case DS18B20_FAIL_TOUT:
                puts( "Timeout") ;
                break ;
            case DS18B20_FAIL_CRC:
                puts( "CRC Error") ;
            }

            ds18b20_convert() ; /* start temperature conversion */
        }
}

/* end of ds18b20main.c */
