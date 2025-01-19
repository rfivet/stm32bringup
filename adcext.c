/* adcext.c -- read external resistor value using ADC */
/* Copyright (c) 2021-2025 Renaud Fivet */

#include <limits.h>
#include <stdio.h>
#include "system.h"	/* uptime, yield(), adc_init(), adc_convert() */

#define RREF 10010  /* Rref is 10kOhm, measured @ 10.01 kOhm */

int main( void) {
    unsigned last = 0 ;
    const unsigned short *calp ;        /* TS_CAL, VREFINT_CAL */

/* Initialize ADC and fetch calibration values */
    calp = adc_init( 2 | (1 << 17)) ;   /* ADC read on GPIOA1 and VREF */
    short Vcal = calp[ 1] ;             /* VREFINT_CAL */

    printf( "factory calibration: %u, %u, %u\n", calp[ 1], calp[ 0], calp[5]) ;

    for( ;;)
        if( uptime == last)
            yield() ;
        else {
            short Rsample, Vsample ;

            last = uptime ;
            Vsample = adc_convert() ;
            Rsample = adc_convert() ;
            printf( "%i, %i, %i, ", Vcal, Vsample, Rsample) ;
            int res = Rsample ? RREF * 4095 / Rsample - RREF : INT_MAX ;
            Vsample = 3300 * Vcal / Vsample ;
            printf( "%i, %i.%i\n", res, Vsample / 1000, Vsample % 1000) ;
        }
}

/* end of adcext.c */
