/* adccalib.c -- ADC calibration of internal temperature sensor */
/* Copyright (c) 2020-2021 Renaud Fivet */

#include <stdio.h>
#include "system.h"     /* uptime, adc_vnt() */
#include "ds18b20.h"    /* ds18b20_() */

#define ABS( i) ((i < 0) ? -i : i)

static void track( short *minp, short *maxp, short val) {
    if( val < *minp)
        *minp = val ;

    if( val > *maxp)
        *maxp = val ;
}

int main( void) {
    unsigned last = 0 ;
    short calV, calC ;
    short minC, maxC ;  /* Track temperature from DS18B20 */
    short minV, maxV ;  /* Track ADC raw Vref */
    short minT, maxT ;  /* Track ADC raw Tsense */

    minC = minV = minT = 0x7FFF ;
    maxC = maxV = maxT = -32768 ;

/* Initialize ADC and fetch calibration values */
    adc_vnt( VNT_INIT, &calV, &calC) ;
    printf( "%u, %u\n", calV, calC) ;
    int tconst = 6660 * calV / calC ;

/* Initialize DS18B20 and initiate temperature conversion */
    ds18b20_init() ;
    ds18b20_resolution( 12) ;   /* Set highest resolution: 12 bits */
    ds18b20_convert() ;         /* start temperature conversion */

/* main acquisition loop, reads samples every seconds */
    for( ;;)
        if( uptime == last)
            yield() ;
        else {
            short Vsample, Csample ;

            last = uptime ;

        /* Track DS18B20 temperature readings */
            switch( ds18b20_fetch( &Csample)) {
            case DS18B20_SUCCESS:
                track( &minC, &maxC, Csample) ;
                printf( "%i.%i, %i, %i, ", Csample / 10, ABS( Csample % 10),
                                                                minC, maxC) ;
                break ;
            case DS18B20_FAIL_TOUT:
                printf( "Timeout, ") ;
                break ;
            case DS18B20_FAIL_CRC:
                printf( "CRC Error, ") ;
            }

            ds18b20_convert() ; /* start temperature conversion */

        /* Track Internal Temperature Sensor readings */
            adc_vnt( VNT_RAW, &Vsample, &Csample) ;
            track( &minV, &maxV, Vsample) ;
            track( &minT, &maxT, Csample) ;
            printf( "%i, %i, %i, %i, %i, %i, %i, %i, ",
                    calV, Vsample, minV, maxV,
                    calC, Csample, minT, maxT) ;
            Csample = 3630 - (1 + tconst * Csample / Vsample) / 2 ;
            Vsample = (660 * calV / Vsample + 1) / 2 ;
            printf( "%i.%i, %i.%i\n",   Vsample / 100, ABS( Vsample % 100),
                                        Csample / 10, ABS( Csample % 10)) ;
        }
}

/* end of adccalib.c */
