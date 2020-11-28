/* usart1tx.c -- Asynchronous Serial Transmission on USART1
** Copyright (c) 2020 Renaud Fivet
**
** SysClck 8MHz HSI based, baudrate 9600, Busy wait transmission
*/

#define SYSTICK                 ((volatile long *) 0xE000E010)
#define SYSTICK_CSR             SYSTICK[ 0]
#define SYSTICK_RVR             SYSTICK[ 1]
#define SYSTICK_CVR             SYSTICK[ 2]

#define CAT( a, b) a##b
#define HEXA( a) CAT( 0x, a)
#define RCC                     ((volatile long *) 0x40021000)
#define RCC_AHBENR              RCC[ 5]
#define RCC_AHBENR_IOP( h)      (1 << (17 + HEXA( h) - 0xA))
#define RCC_APB2ENR             RCC[ 6]
#define RCC_APB2ENR_USART1EN    0x00004000  /* 14: USART1 clock enable */

#define GPIOA                   ((volatile long *) 0x48000000)
#define GPIOB                   ((volatile long *) 0x48000400)
#define GPIO( x) CAT( GPIO, x)
#define MODER   0
#define ODR     5
#define AFRH    9

#define USART1                  ((volatile long *) 0x40013800)
#define CR1     0               /* Config */
#define BRR     3               /* Baudrate */
#define ISR     7               /* Interrupt and Status */
#define TDR     10              /* Transmit Data */
#define USART_CR1_TE    8           /* 3: Transmit Enable */
#define USART_CR1_RE    4           /* 2: Receive Enable */
#define USART_CR1_UE    1           /* 0: USART Enable */
#define USART_ISR_TXE   (1 << 7)    /* 7: Transmit Data Register Empty */

/* user LED ON when PA4 is high */
#define LED_IOP A
#define LED_PIN 4
#define LED_ON  1

void kputc( unsigned char c) {
    static unsigned char lastc ;

    if( c == '\n' && lastc != '\r')
        kputc( '\r') ;

/* Active wait while transmit register is full */
    while( (USART1[ ISR] & USART_ISR_TXE) == 0) ;

    USART1[ TDR] = c ;
    lastc = c ;
}

int puts( const char *s) {
    while( *s)
        kputc( *s++) ;

    kputc( '\n') ;
    return 0 ;
}

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

/* USART1 9600 8N1 */
    RCC_AHBENR |= RCC_AHBENR_IOP( A) ;  /* Enable GPIOA periph */
    GPIOA[ MODER] |= 0x0A << (9 * 2) ;  /* PA9-10 ALT 10, over default 00 */
    GPIOA[ AFRH] |= 0x110 ;             /* PA9-10 AF1 0001, over default 0000 */
    RCC_APB2ENR |= RCC_APB2ENR_USART1EN ;
    USART1[ BRR] = 8000000 / 9600 ;     /* PCLK [8MHz] */
    USART1[ CR1] |= USART_CR1_UE | USART_CR1_TE ;   /* Enable USART & Tx */

    return 0 ;
}

/* end of usart1tx.c */
