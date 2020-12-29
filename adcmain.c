/* adcmain.c -- ADC reading of reference voltage and temperature sensor */
/* Copyright (c) 2020 Renaud Fivet */

#include <stdio.h>
#include "system.h"

#define RAW

int main( void) {
    unsigned last = 0 ;
    short calV, calC ;

/* Initialize ADC and fetch calibration values */
    adc_vnt( VNT_INIT, &calV, &calC) ;
#ifdef RAW
    printf( "%u, %u\n", calV, calC) ;
#endif

    for( ;;)
        if( uptime == last)
            yield() ;
        else {
            short Vsample, Csample ;

            last = uptime ;
#ifdef RAW
            adc_vnt( VNT_RAW, &Vsample, &Csample) ;
            printf( "%i, %i, %i, %i, ", calV, Vsample, calC, Csample) ;
            Vsample = 330 * calV / Vsample ;
            Csample = 850 + (1500 - (int) Csample) * 10 / 4 ;
#else
            adc_vnt( VNT_VNC, &Vsample, &Csample) ;
#endif
            printf( "%i.%i, %i.%i\n", Vsample / 100, Vsample % 100,
                                                Csample / 10, Csample % 10) ;
        }
}

/* end of adcmain.c */
