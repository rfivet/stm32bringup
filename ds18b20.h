/* ds18b20.h -- 1-Wire temperature sensor */
/* Copyright (c) 2020 Renaud Fivet */

typedef enum {
    DS18B20_SUCCESS,
    DS18B20_FAIL_TOUT,
    DS18B20_FAIL_CRC
} ds18b20_retv_t ;

void ds18b20_init( void) ;
ds18b20_retv_t ds18b20_resolution( unsigned res) ;  /* 9..12 bits  */
ds18b20_retv_t ds18b20_convert( void) ;
ds18b20_retv_t ds18b20_fetch( short *deciCtemp) ;/* -550~1250 = -55.0~125.0 C */
ds18b20_retv_t ds18b20_read( short *deciCtemp) ; /* -550~1250 = -55.0~125.0 C */

/* end of ds18b20.h */
