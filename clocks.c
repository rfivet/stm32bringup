/* clocks.c -- system layer
** Copyright (c) 2020 Renaud Fivet
**
** clocks configuration: HSI, HSE, PLL HSI, PLL HSE
** implements system.h interface
** uptime = seconds elapsed since boot
** SysClck 8MHz HSI based, baudrate 9600, Busy wait transmission
** user LED toggled every second
** SysTick interrupt every second
*/

#include "system.h" /* implements system.h */

#define SYSTICK                 ((volatile long *) 0xE000E010)
#define SYSTICK_CSR             SYSTICK[ 0]
#define SYSTICK_RVR             SYSTICK[ 1]
#define SYSTICK_CVR             SYSTICK[ 2]

#define CAT( a, b) a##b
#define HEXA( a) CAT( 0x, a)
#define RCC                     ((volatile long *) 0x40021000)

#define RCC_CR          RCC[ 0]
#define RCC_CR_HSION    0x00000001  /*  1: Internal High Speed clock enable */
#define RCC_CR_HSEON    0x00010000  /* 16: External High Speed clock enable */
#define RCC_CR_HSERDY   0x00020000  /* 17: External High Speed clock ready flag */
#define RCC_CR_PLLON    0x01000000  /* 24: PLL enable */
#define RCC_CR_PLLRDY   0x02000000  /* 25: PLL clock ready flag */

#define RCC_CFGR            RCC[ 1]
#define RCC_CFGR_SW_MSK     0x00000003  /* 1-0: System clock SWitch Mask */
#define RCC_CFGR_SW_HSE     0x00000001  /* 1-0: Switch to HSE as system clock */
#define RCC_CFGR_SW_PLL     0x00000002  /* 1-0: Switch to PLL as system clock */
#define RCC_CFGR_SWS_MSK    0x0000000C  /* 3-2: System clock SWitch Status Mask */
#define RCC_CFGR_SWS_HSE    0x00000004  /* 3-2: HSE used as system clock */
#define RCC_CFGR_SWS_PLL    0x00000008  /* 3-2: PLL used as system clock */
#define RCC_CFGR_PLLSRC         0x00010000
#define RCC_CFGR_PLLSRC_HSI     0x00000000      /* HSI / 2 */
#define RCC_CFGR_PLLSRC_HSE     0x00010000      /* HSE */
#define RCC_CFGR_PLLXTPRE       0x00020000
#define RCC_CFGR_PLLXTPRE_DIV1  0x00000000  /* HSE */
#define RCC_CFGR_PLLXTPRE_DIV2  0x00020000  /* HSE / 2 */
#define RCC_CFGR_PLLMUL_MSK     (0x00F << 18)
#define RCC_CFGR_PLLMUL( v)     ((v - 2) << 18)

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

/* user LED ON when PA4 is low */
#define LED_IOP A
#define LED_PIN 4
#define LED_ON  0
/* 8MHz quartz, configure PLL at 24MHz */
#define HSE     8000000
#define PLL     6
#define BAUD    9600

#ifdef PLL
# ifdef HSE
#  define CLOCK HSE / 2 * PLL
# else /* HSI */
#  define CLOCK 8000000 / 2 * PLL
# endif
# if CLOCK < 16000000
# error PLL output below 16MHz
# endif
#else
# ifdef HSE
#  define CLOCK HSE
# else /* HSI */
#  define CLOCK 8000000
# endif
#endif

#if CLOCK > 48000000
# error clock frequency exceeds 48MHz
#endif

#if CLOCK % BAUD
# warning baud rate not accurate at that clock frequency
#endif

void kputc( unsigned char c) {  /* character output */
    static unsigned char lastc ;

    if( c == '\n' && lastc != '\r')
        kputc( '\r') ;

/* Active wait while transmit register is full */
    while( (USART1[ ISR] & USART_ISR_TXE) == 0) ;

    USART1[ TDR] = c ;
    lastc = c ;
}

int kputs( const char s[]) {    /* string output */
    int cnt = 0 ;
    int c ;

    while( (c = *s++) != 0) {
        kputc( c) ;
        cnt += 1 ;
    }

    return cnt ;
}

void yield( void) {             /* give way */
    __asm( "WFI") ; /* Wait for System Tick Interrupt */
}

volatile unsigned uptime ;      /* seconds elapsed since boot */

#ifdef LED_ON
static void userLEDtoggle( void) {
    GPIO( LED_IOP)[ ODR] ^= 1 << LED_PIN ;   /* Toggle User LED */
}
#endif

void SysTick_Handler( void) {
    uptime += 1 ;
#ifdef LED_ON
    userLEDtoggle() ;
#endif
}

int init( void) {
/* By default SYSCLK == HSI [8MHZ] */

#ifdef HSE
/* Start HSE clock (8 MHz external oscillator) */
    RCC_CR |= RCC_CR_HSEON ;
/* Wait for oscillator to stabilize */
    do {} while( (RCC_CR & RCC_CR_HSERDY) == 0) ;
#endif

#ifdef PLL
/* Setup PLL HSx/2 * 6 [24MHz] */
    /* Default 0: PLL HSI/2 src, PLL MULL * 2 */
# ifdef HSE
    RCC_CFGR = RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLXTPRE_DIV2 ;
# endif
    RCC_CFGR |= RCC_CFGR_PLLMUL( PLL) ;
    RCC_CR |= RCC_CR_PLLON ;
    do {} while( (RCC_CR & RCC_CR_PLLRDY) == 0) ;   /* Wait for PLL */

/* Switch to PLL as system clock SYSCLK == PLL [24MHz] */
    RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_SW_MSK) | RCC_CFGR_SW_PLL ;
    do {} while( (RCC_CFGR & RCC_CFGR_SWS_MSK) != RCC_CFGR_SWS_PLL) ;
#else
# ifdef HSE
/* Switch to HSE as system clock SYSCLK == HSE [8MHz] */
    RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_SW_MSK) | RCC_CFGR_SW_HSE ;
    do {} while( (RCC_CFGR & RCC_CFGR_SWS_MSK) != RCC_CFGR_SWS_HSE) ;
# endif
#endif

#ifdef HSE
/* Switch off HSI */
    RCC_CR &= ~RCC_CR_HSION ;
#endif

/* SYSTICK */
    SYSTICK_RVR = CLOCK / 8 - 1 ;   /* HBA / 8 */
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
    userLEDtoggle() ;
# endif
#endif

/* USART1 9600 8N1 */
    RCC_AHBENR |= RCC_AHBENR_IOP( A) ;  /* Enable GPIOA periph */
    GPIOA[ MODER] |= 0x0A << (9 * 2) ;  /* PA9-10 ALT 10, over default 00 */
    GPIOA[ AFRH] |= 0x110 ;             /* PA9-10 AF1 0001, over default 0000 */
    RCC_APB2ENR |= RCC_APB2ENR_USART1EN ;
    USART1[ BRR] = CLOCK / BAUD ;       /* PCLK is default source */
    USART1[ CR1] |= USART_CR1_UE | USART_CR1_TE ;   /* Enable USART & Tx */

    kputs(
#ifdef PLL
        "PLL"
#endif
#ifdef HSE
        "HSE"
#else
        "HSI"
#endif
        "\n") ;

    return 0 ;
}

/* end of clocks.c */
