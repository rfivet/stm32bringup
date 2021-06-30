/* adc.c -- system layer
** Copyright (c) 2020-2021 Renaud Fivet
**
** ADC for temperature sensor and Vrefint
** gpioa low level API and usleep()
** interrupt based serial transmission
** clocks configuration: HSI, HSE, PLL HSI, PLL HSE
** implements system.h interface: uptime, init(), kputc(), kputs(), yield()
** uptime = seconds elapsed since boot
** Serial tx, SysClck 8MHz HSI based, baudrate 9600, Busy wait transmission
** user LED toggled every second
** SysTick interrupt every second
*/

#include "system.h" /* implements system.h */


/** CORE **********************************************************************/

#define SYSTICK                 ((volatile unsigned long *) 0xE000E010)
#define SYSTICK_CSR             SYSTICK[ 0]
#define SYSTICK_RVR             SYSTICK[ 1]
#define SYSTICK_CVR             SYSTICK[ 2]

#define NVIC                    ((volatile long *) 0xE000E100)
#define NVIC_ISER               NVIC[ 0]
#define unmask_irq( idx)        NVIC_ISER = 1 << idx
#define USART1_IRQ_IDX          27


/** PERIPH ********************************************************************/

#define CAT( a, b) a##b
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
#define RCC_AHBENR_IOPn( n)     (1 << (17 + n))
#define RCC_AHBENR_IOPh( h)     RCC_AHBENR_IOPn( CAT( 0x, h) - 0xA)

#define RCC_APB2ENR             RCC[ 6]
#define RCC_APB2ENR_USART1EN    0x00004000  /* 14: USART1 clock enable */
#define RCC_APB2ENR_ADCEN       0x00000200  /*  9: ADC clock enable */

#define RCC_CR2                 RCC[ 13]
#define RCC_CR2_HSI14ON         0x00000001  /*  1: HSI14 clock enable */
#define RCC_CR2_HSI14RDY        0x00000002  /*  2: HSI14 clock ready */


#define GPIOA                   ((volatile long *) 0x48000000)
#define GPIOB                   ((volatile long *) 0x48000400)
#define GPIO( x) CAT( GPIO, x)
#define MODER   0
#define IDR     4
#define ODR     5
#define AFRH    9

#define ADC                     ((volatile long *) 0x40012400)
#define ADC_ISR                 ADC[ 0]
#define ADC_ISR_ADRDY           1   /* 0: ADC Ready */
#define ADC_ISR_EOC             4   /* 2: End Of Conversion flag */

#define ADC_CR                  ADC[ 2]
#define ADC_CR_ADEN             1   /* 0: ADc ENable command */
#define ADC_CR_ADSTART          4   /* 2: ADC Start Conversion command */
#define ADC_CR_ADCAL            (1 << 31)   /* 31: ADC Start Calibration cmd */

#define ADC_CFGR1               ADC[ 3]     /* Configuration Register 1 */
#define ADC_CFGR1_SCANDIR       4           /*  2: Scan sequence direction */
#define ADC_CFGR1_DISCEN        (1 << 16)   /* 16: Enable Discontinuous mode */

#define ADC_CFGR2               ADC[ 4]     /* Configuration Register 2 */
#define ADC_CFGR2_CKMODE        (3 << 30)   /* 31-30: Clock Mode Mask   */
                                            /* 31-30: Default 00 HSI14  */
#define ADC_CFGR2_PCLK2         (1 << 30)   /* 31-30: PCLK/2 */
#define ADC_CFGR2_PCLK4         (2 << 30)   /* 31-30: PCLK/4 */

#define ADC_SMPR                ADC[ 5]     /* Sampling Time Register */
#define ADC_CHSELR              ADC[ 10]    /* Channel Selection Register */
#define ADC_DR                  ADC[ 16]    /* Data Register */
#define ADC_CCR                 ADC[ 194]   /* Common Configuration Register */
#define ADC_CCR_VREFEN          (1 << 22)   /* 22: Vrefint Enable */
#define ADC_CCR_TSEN            (1 << 23)   /* 23: Temperature Sensor Enable */

#define USART1                  ((volatile long *) 0x40013800)
#define CR1     0               /* Config Register */
#define BRR     3               /* BaudRate Register */
#define ISR     7               /* Interrupt and Status Register */
#define TDR     10              /* Transmit Data Register*/
#define USART_CR1_TXEIE (1 << 7)    /* 7: TDR Empty Interrupt Enable */
#define USART_CR1_TE    8           /* 3: Transmit Enable */
#define USART_CR1_RE    4           /* 2: Receive Enable */
#define USART_CR1_UE    1           /* 0: USART Enable */
#define USART_ISR_TXE   (1 << 7)    /* 7: Transmit Data Register Empty */


/** SYSTEM MEMORY *************************************************************/
/* STM32F030 calibration addresses (at 3.3V and 30C) */
#define TS_CAL1                 ((unsigned short *) 0x1FFFF7B8)
#define VREFINT_CAL             ((unsigned short *) 0x1FFFF7BA)


/* user LED ON when PA4 is low */
#define LED_IOP A
#define LED_PIN 4
#define LED_ON  0
/* 8MHz quartz, configure PLL at 28MHz */
//#define HSE     8000000
#define PLL     7
#define BAUD    9600
//#define HSI14 1

#ifdef PLL
# ifdef HSE
#  define CLOCK HSE / 2 * PLL
# else /* HSI */
#  define CLOCK 8000000 / 2 * PLL
# endif
# if CLOCK < 16000000
#  error PLL output below 16MHz
# endif
#elif defined( HSE)
# define CLOCK HSE
#else /* HSI */
# define CLOCK 8000000
#endif

#if CLOCK > 48000000
# error clock frequency exceeds 48MHz
#endif

#if CLOCK % BAUD
# warning baud rate not accurate at that clock frequency
#endif

static unsigned char txbuf[ 8] ; // best if size is a power of 2 for cortex-M0
#define TXBUF_SIZE (sizeof txbuf / sizeof txbuf[ 0])
static unsigned char            txbufin ;
static volatile unsigned char   txbufout ;

void USART1_Handler( void) {
    if( txbufout == txbufin) {
    /* Empty buffer => Disable TXEIE */
        USART1[ CR1] &= ~USART_CR1_TXEIE ;
    } else {
        static unsigned char lastc ;
        unsigned char c ;

        c = txbuf[ txbufout] ;
        if( c == '\n' && lastc != '\r')
            c = '\r' ;
        else
            txbufout = (txbufout + 1) % TXBUF_SIZE ;

        USART1[ TDR] = c ;
        lastc = c ;
    }
}

void kputc( unsigned char c) {  /* character output */
    int nextidx ;

/* Wait if buffer full */
    nextidx = (txbufin + 1) % TXBUF_SIZE ;
    while( nextidx == txbufout)
        yield() ;

    txbuf[ txbufin] = c ;
    txbufin = nextidx ;
/* Trigger transmission by enabling interrupt */
    USART1[ CR1] |= USART_CR1_TXEIE ;
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


void usleep( unsigned usecs) {      /* wait at least usec µs */
#if CLOCK / 8000000 < 1
# error HCLK below 8 MHz
#elif CLOCK % 8000000
# warning HCLK is not multiple of 8 MHz
#endif
    usecs = SYSTICK_CVR - (CLOCK / 8000000 * usecs) ;
    while( SYSTICK_CVR > usecs) ;
}


/* GPIOA low level API ********************************************************/

void gpioa_input( int pin) {        /* Configure GPIOA pin as input */
    GPIOA[ MODER] &= ~(3 << (pin * 2)) ;    /* Apin as input [00] */
}

void gpioa_output( int pin) {       /* Configure GPIOA pin as output */
    GPIOA[ MODER] |= 1 << (pin * 2) ;       /* Apin output (over [00]) */
}

iolvl_t gpioa_read( int pin) {      /* Read level of GPIOA pin */
    return LOW != (GPIOA[ IDR] & (1 << pin)) ;
}


static void adc_init( void) {
/* Enable ADC peripheral */
    RCC_APB2ENR |= RCC_APB2ENR_ADCEN ;
/* Setup ADC sampling clock */
#ifdef HSI14
    RCC_CR2 |= RCC_CR2_HSI14ON ;                    /* Start HSI14 clock */
    do {} while( !( RCC_CR2 & RCC_CR2_HSI14RDY)) ;  /* Wait for stable clock */
/* Select HSI14 as sampling clock for ADC */
//  ADC_CFGR2 &= ~ADC_CFGR2_CKMODE ;    /* Default 00 == HSI14 */
#else
/* Select PCLK/2 as sampling clock for ADC */
    ADC_CFGR2 |= ADC_CFGR2_PCLK2 ;          /* 01 PCLK/2 Over default 00 */
//  ADC_CFGR2 |= ADC_CFGR2_PCLK4 ;          /* 10 PCLK/4 Over default 00 */
#endif

/* Calibration */
    ADC_CR |= ADC_CR_ADCAL ;
    do {} while( ADC_CR & ADC_CR_ADCAL) ;   /* Wait end of calibration */

/* Enable Command (below Work Around from Errata necessary with PCLK/4) */
    do {
        ADC_CR |= ADC_CR_ADEN ;
    } while( !( ADC_ISR & ADC_ISR_ADRDY)) ;

/* Select inputs, precision and scan direction */
    ADC_CHSELR = 3 << 16 ;  /* Channel 16: temperature, Channel 17: Vrefint */
    ADC_SMPR = 7 ;
    ADC_CCR |= ADC_CCR_TSEN | ADC_CCR_VREFEN ;
/* Default scan direction (00) is Temperature before Voltage */
//  ADC_CFGR1 &= ~ADC_CFGR1_SCANDIR ;   /* Default 0 is low to high */
    ADC_CFGR1 |= ADC_CFGR1_DISCEN ;     /* Enable Discontinuous mode */
}

static unsigned adc_convert( void) {
/* Either only one channel in sequence or Discontinuous mode ON */
    ADC_CR |= ADC_CR_ADSTART ;              /* Start ADC conversion */
    do {} while( ADC_CR & ADC_CR_ADSTART) ; /* Wait for start command cleared */
    return ADC_DR ;
}

void adc_vnt( vnt_cmd_t cmd, short *ptrV, short *ptrC) {
    if( cmd == VNT_INIT)
        adc_init() ;

    if( cmd <= VNT_CAL) {
    /* Calibration Values */
        *ptrV = *VREFINT_CAL ;
        *ptrC = *TS_CAL1 ;
        return ;
    }

/* ADC Conversion */
    *ptrC = adc_convert() ;
    *ptrV = adc_convert() ;
    
    if( cmd == VNT_VNC) {
		*ptrC = 300 + (*TS_CAL1 - *ptrC * *VREFINT_CAL / *ptrV) * 10000 / 5336 ;
        *ptrV = 330 * *VREFINT_CAL / *ptrV ;
    }
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
    RCC_CFGR =
# ifdef HSE
                RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLXTPRE_DIV2 |
# endif
                RCC_CFGR_PLLMUL( PLL) ;
    RCC_CR |= RCC_CR_PLLON ;
    do {} while( (RCC_CR & RCC_CR_PLLRDY) == 0) ;   /* Wait for PLL */

/* Switch to PLL as system clock SYSCLK == PLL [24MHz] */
    RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_SW_MSK) | RCC_CFGR_SW_PLL ;
    do {} while( (RCC_CFGR & RCC_CFGR_SWS_MSK) != RCC_CFGR_SWS_PLL) ;
#elif defined( HSE)
/* Switch to HSE as system clock SYSCLK == HSE [8MHz] */
    RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_SW_MSK) | RCC_CFGR_SW_HSE ;
    do {} while( (RCC_CFGR & RCC_CFGR_SWS_MSK) != RCC_CFGR_SWS_HSE) ;
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
    RCC_AHBENR |= RCC_AHBENR_IOPh( LED_IOP) ;       /* Enable IOPx periph */
    GPIO( LED_IOP)[ MODER] |= 1 << (LED_PIN * 2) ;  /* LED_IO Output [01],
                                                    ** over default 00 */
    /* OTYPER Push-Pull by default */
    /* Pxn output default LOW at reset */
# if LED_ON
    userLEDtoggle() ;
# endif
#endif

/* USART1 9600 8N1 */
    RCC_AHBENR |= RCC_AHBENR_IOPh( A) ; /* Enable GPIOA periph */
    GPIOA[ MODER] |= 0x0A << (9 * 2) ;  /* PA9-10 ALT 10, over default 00 */
    GPIOA[ AFRH] |= 0x110 ;             /* PA9-10 AF1 0001, over default 0000 */
    RCC_APB2ENR |= RCC_APB2ENR_USART1EN ;
    USART1[ BRR] = CLOCK / BAUD ;       /* PCLK is default source */
    USART1[ CR1] |= USART_CR1_UE | USART_CR1_TE ;   /* Enable USART & Tx */

/* Unmask USART1 irq */
    unmask_irq( USART1_IRQ_IDX) ;

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

/* end of adc.c */
