/* adcmain.c -- ADC reading of reference voltage and temperature sensor */
/* Copyright (c) 2020-2025 Renaud Fivet */

#include <stdio.h>
#include "system.h"

#define RAW

#define TS_CAL2 ((const short *) 0x1FFFF7C2)
//#define USER0   ((const unsigned char *) 0x1FFFF804)

int main( void) {
    unsigned last = 0 ;
    short calV, calC ;

/* Initialize ADC and fetch calibration values */
    adc_vnt( VNT_INIT, &calV, &calC) ;
#ifdef RAW
    printf( "%u, %u\n", calV, calC) ;

    int baseC = 300 ;
# ifdef USER0
    if( 0xFF == (USER0[ 0] ^ USER0[ 1]))
        baseC = USER0[ 0] * 10 ;
# endif
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
            Csample = baseC + (calC - (int) Csample * calV / Vsample)
# ifdef TS_CAL2
                                                    * 800 / (calC - *TS_CAL2) ;
# else
                                                                * 10000 / 5336 ;
# endif
            Vsample = 3300 * calV / Vsample ;
#else
            adc_vnt( VNT_VNC, &Vsample, &Csample) ;
#endif
            printf( "%i.%i, %i.%i\n", Vsample / 1000, Vsample % 1000,
                                                Csample / 10, Csample % 10) ;
        }
}

/* end of adcmain.c */
