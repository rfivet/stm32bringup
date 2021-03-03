/* stm32f030xx.h -- STM32F030xx specific mapping */
/* Copyright (c) 2021 Renaud Fivet               */

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
#define RCC_APB2ENR_SYSCFGEN    0x00000001  /*  1: SYSCFG clock enable */

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

#define SYSCFG                  ((volatile long *) 0x40010000)
#define SYSCFG_CFGR1            SYSCFG[ 0]

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
#define TS_CAL                  ((unsigned short *) 0x1FFFF7B8)
#define VREFINT_CAL             ((unsigned short *) 0x1FFFF7BA)

/* end of stm32f030xx.h */
