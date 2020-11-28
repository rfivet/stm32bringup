/* board.c -- board mapping abstraction
** Copyright (c) 2020 Renaud Fivet
*/

#define SYSTICK             ((volatile long *) 0xE000E010)
#define SYSTICK_CSR         SYSTICK[ 0]
#define SYSTICK_RVR         SYSTICK[ 1]
#define SYSTICK_CVR         SYSTICK[ 2]

#define CAT( a, b) a##b
#define HEXA( a) CAT( 0x, a)
#define RCC                 ((volatile long *) 0x40021000)
#define RCC_AHBENR          RCC[ 5]
#define RCC_AHBENR_IOP( h)  (1 << (17 + HEXA( h) - 0xA))

#define GPIOA               ((volatile long *) 0x48000000)
#define GPIOB               ((volatile long *) 0x48000400)
#define GPIO( x) CAT( GPIO, x)
#define MODER   0
#define ODR     5

/* user LED ON when PA4 is high */
#define LED_IOP A
#define LED_PIN 4
#define LED_ON  1

void SysTick_Handler( void) {
#ifdef LED_ON
    GPIO( LED_IOP)[ ODR] ^= 1 << LED_PIN ;   /* Toggle User LED */
#endif
}

int init( void) {
/* By default SYSCLK == HSI [8MHZ] */

/* SYSTICK */
    SYSTICK_RVR = 1000000 - 1 ;     /* HBA / 8 */
    SYSTICK_CVR = 0 ;
    SYSTICK_CSR = 3 ;               /* HBA / 8, Interrupt ON, Enable */
    /* SysTick_Handler will execute every 1s from now on */

#ifdef LED_ON
/* User LED ON */
    RCC_AHBENR |= RCC_AHBENR_IOP( LED_IOP) ;        /* Enable IOPx periph */
    GPIO( LED_IOP)[ MODER] |= 1 << (LED_PIN * 2) ;  /* LED_IO Output [01],
                                                    ** over default 00 */
    /* OTYPER Push-Pull by default */
    /* Pxn output default LOW at reset */
# if LED_ON
    SysTick_Handler() ;
# endif
#endif
    return 0 ;
}

/* end of board.c */
