#include <stdint.h>
#include "stm32g0xx.h"

// BASE ADDRESSES
#define RCC_BASE    0x40021000UL
#define GPIOA_BASE  0x50000000UL
#define GPIOB_BASE  0x50000400UL
#define GPIOC_BASE  0x50000800UL
#define TIM2_BASE   0x40000000UL
#define EXTI_BASE   0x40021800UL
#define ADC_BASE    0x40012400UL
#define ADC_CMN     0x40012708UL

// RCC REGISTERS
#define RCC_IOPENR   (*(volatile uint32_t *)(RCC_BASE + 0x34))
#define RCC_APBENR1  (*(volatile uint32_t *)(RCC_BASE + 0x3C))
#define RCC_APBENR2  (*(volatile uint32_t *)(RCC_BASE + 0x40))

// GPIO REGISTERS
#define GPIOA_MODER  (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_ODR    (*(volatile uint32_t *)(GPIOA_BASE + 0x14))
#define GPIOB_MODER  (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_ODR    (*(volatile uint32_t *)(GPIOB_BASE + 0x14))
#define GPIOC_MODER  (*(volatile uint32_t *)(GPIOC_BASE + 0x00))

// TIM2 REGISTERS
#define TIM2_CR1     (*(volatile uint32_t *)(TIM2_BASE + 0x00))
#define TIM2_DIER    (*(volatile uint32_t *)(TIM2_BASE + 0x0C))
#define TIM2_SR      (*(volatile uint32_t *)(TIM2_BASE + 0x10))
#define TIM2_PSC     (*(volatile uint32_t *)(TIM2_BASE + 0x28))
#define TIM2_ARR     (*(volatile uint32_t *)(TIM2_BASE + 0x2C))

// EXTI REGISTERS
#define EXTI_RTSR1   (*(volatile uint32_t *)(EXTI_BASE + 0x00))
#define EXTI_IMR1    (*(volatile uint32_t *)(EXTI_BASE + 0x80))
#define EXTI_RPR1    (*(volatile uint32_t *)(EXTI_BASE + 0x0C))
#define EXTI_EXTICR4 (*(volatile uint32_t *)(EXTI_BASE + 0x6C))

// ADC REGISTERS
#define ADC_ISR      (*(volatile uint32_t *)(ADC_BASE + 0x00))
#define ADC_IER      (*(volatile uint32_t *)(ADC_BASE + 0x04))
#define ADC_CR       (*(volatile uint32_t *)(ADC_BASE + 0x08))
#define ADC_CFGR1    (*(volatile uint32_t *)(ADC_BASE + 0x0C))
#define ADC_CFGR2    (*(volatile uint32_t *)(ADC_BASE + 0x10))
#define ADC_SMPR     (*(volatile uint32_t *)(ADC_BASE + 0x14))
#define ADC_CHSELR   (*(volatile uint32_t *)(ADC_BASE + 0x28))
#define ADC_DR       (*(volatile uint32_t *)(ADC_BASE + 0x40))
#define ADC_CCR      (*(volatile uint32_t *)(ADC_CMN   + 0x00))

// GLOBAL VARIABLES
volatile uint32_t ms_count    = 0;
volatile uint32_t led2_count  = 0;
volatile uint8_t  blinking    = 1;
volatile uint32_t adcValue    = 0;
volatile uint32_t blink_delay = 500;

// TIM2 ISR — fires every 1ms
void TIM2_IRQHandler(void)
{
    TIM2_SR &= ~(1 << 0);  // clear flag first

    if (blinking)
    {
        ms_count++;
        led2_count++;

        // LED1 on PA5 — always 1Hz
        if (ms_count >= 500)
        {
            ms_count = 0;
            GPIOA_ODR ^= (1 << 5);
        }

        // LED2 on PB0 — rate controlled by potentiometer
        if (led2_count >= blink_delay)
        {
            led2_count = 0;
            GPIOB_ODR ^= (1 << 0);
        }
    }
}

// BUTTON ISR — fires when PC13 pressed
void EXTI4_15_IRQHandler(void)
{
    EXTI_RPR1 = (1 << 13);  // clear pending flag

    blinking = !blinking;

    // turn both LEDs off immediately when stopped
    if (!blinking)
    {
        GPIOA_ODR &= ~(1 << 5);
        GPIOB_ODR &= ~(1 << 0);
    }
}

// ADC ISR — fires when potentiometer reading is ready
void ADC1_COMP_IRQHandler(void)
{
    adcValue = ADC_DR;          // read value (0-4095)
    ADC_ISR &= ~(1 << 2);      // clear EOC flag

    // linear interpolation: 0 → 100ms (5Hz), 4095 → 500ms (1Hz)
    blink_delay = 100 + (adcValue * 400) / 4095;

    led2_count = 0;             // reset counter so new delay applies immediately
    ADC_CR |= (1 << 2);        // start next conversion (ADSTART)
}

int main(void)
{
    // ── ENABLE CLOCKS ──────────────────────────────────────────
    RCC_IOPENR  |= (1 << 0);   // GPIOA
    RCC_IOPENR  |= (1 << 1);   // GPIOB
    RCC_IOPENR  |= (1 << 2);   // GPIOC
    RCC_APBENR1 |= (1 << 0);   // TIM2
    RCC_APBENR2 |= (1 << 0);   // SYSCFG
    RCC_APBENR2 |= (1 << 20);  // ADC

    // ── GPIO SETUP ─────────────────────────────────────────────
    // PA5 as output (LED1)
    GPIOA_MODER &= ~(3 << 10);
    GPIOA_MODER |=  (1 << 10);

    // PB0 as output (LED2)
    GPIOB_MODER &= ~(3 << 0);
    GPIOB_MODER |=  (1 << 0);

    // PC13 as input (button)
    GPIOC_MODER &= ~(3 << 26);

    // PA0 as analog input (potentiometer) — FIX: clear then set
    GPIOA_MODER &= ~(3 << 0);  // clear bits first
    GPIOA_MODER |=  (3 << 0);  // set to analog mode (11)

    // ── TIM2 SETUP ─────────────────────────────────────────────
    TIM2_CR1  &= ~(1 << 4);    // count up mode
    TIM2_PSC   = 15;            // 16MHz / 16 = 1MHz
    TIM2_ARR   = 999;           // overflow every 1ms
    TIM2_DIER |= (1 << 0);     // enable update interrupt
    NVIC_SetPriority(TIM2_IRQn, 1);
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2_CR1  |= (1 << 0);     // start timer

    // ── BUTTON SETUP (PC13 EXTI) ───────────────────────────────
    EXTI_EXTICR4 |= (0x2 << 8); // PC13 as EXTI source
    EXTI_RTSR1   |= (1 << 13);  // rising edge trigger
    EXTI_IMR1    |= (1 << 13);  // unmask line 13
    NVIC_SetPriority(EXTI4_15_IRQn, 0);
    NVIC_EnableIRQ(EXTI4_15_IRQn);

    // ── ADC SETUP ──────────────────────────────────────────────
    ADC_CFGR1 &= ~(3 << 3);    // 12-bit resolution
    ADC_CFGR1 &= ~(1 << 13);   // single conversion mode (CONT = 0)
    ADC_CFGR2 &= ~(3 << 30);   // asynchronous clock mode
    ADC_CCR   &= ~(0xF << 18); // prescaler = 1
    ADC_CHSELR = (1 << 0);     // channel 0 = PA0
    ADC_SMPR  |= (3 << 0);     // sampling time 12.5 cycles

    // FIX: calibrate before enabling
    ADC_CR |= (1 << 31);               // start calibration (ADCAL)
    while (ADC_CR & (1 << 31));        // wait for calibration complete

    ADC_IER |= (1 << 2);               // enable EOC interrupt
    ADC_CR  |= (1 << 0);               // enable ADC (ADEN)
    while (!(ADC_ISR & (1 << 0)));     // wait for ADC ready (ADRDY)

    NVIC_SetPriority(ADC1_COMP_IRQn, 2);
    NVIC_EnableIRQ(ADC1_COMP_IRQn);

    ADC_CR |= (1 << 2);                // start first conversion (ADSTART)

    for (;;) {}
}
