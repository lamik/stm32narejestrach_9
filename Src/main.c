/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Mateusz Salamon
 * @brief          : STM32 na Rejestrach
 ******************************************************************************
 ******************************************************************************

 	 	 	 	 https://msalamon.pl
 	 	 	 	 https://sklep.msalamon.pl
 	 	 	 	 https://kursstm32.pl
 	 	 	 	 https://stm32narejestrach.pl

 */
#include "main.h"

// LED LD4 macros
#define LD4_ON GPIOA->BSRR = GPIO_BSRR_BS5
#define LD4_OFF GPIOA->BSRR = GPIO_BSRR_BR5;

// Constants for Software Timer's actions
#define PWM_TIMER 50

#define PC13 (1<<13)

// Tick for System Time
__IO uint32_t Tick;

// LED LD4 configuration
void LD4_Configure(void);

void LED_PwmPinSetup(void);

// Timer 3 configuration
void TIM3_BaseConfiguration(void);
void TIM3_InterruptsConfiguration(void);
void TIM3_Start(void);

void TIM3_PwmConfiguration(void);

// Get current System Time
uint32_t GetSystemTick(void);

// Software Timers variables
uint32_t Timer_PWM;

int main(void)
{
// 1s = 12 000 000
	// 0,001 = X
	SysTick_Config(12000000 / 1000);

	LD4_Configure();
	LED_PwmPinSetup();

	TIM3_BaseConfiguration();
	TIM3_InterruptsConfiguration();

	TIM3_PwmConfiguration();
	TIM3_Start();

	Timer_PWM = GetSystemTick();

    /* Loop forever */
	while(1)
	{
		if((GetSystemTick() - Timer_PWM) > PWM_TIMER)
		{
			Timer_PWM = GetSystemTick();

			if(TIM3->CCR1 < (TIM3->ARR-1))
				TIM3->CCR1 += 1;
			else
				TIM3->CCR1 = 1;
		}
	}
}

void TIM3_BaseConfiguration(void)
{
	// Enable Timer3
	RCC->APBENR1 |= RCC_APBENR1_TIM3EN;

	// Set Timer Prescaler - PSC Register
	TIM3->PSC = (12 - 1); // (12 MHZ/12) = 1 MHZ, 1 tick = 1 us

	// Set ARR Register
	TIM3->ARR = (100 - 1); // 100 * 1 us = 100 us per one timer cycle
}

void TIM3_Start(void)
{
	// Clear counter register
	TIM3->CNT = 0;

	// Enable Timer
	TIM3->CR1 |= TIM_CR1_CEN;
}

void TIM3_InterruptsConfiguration(void)
{
	// Set Update Interrupt
	TIM3->DIER |= TIM_DIER_UIE;

	// Set CC1 Interrupt
	TIM3->DIER |= TIM_DIER_CC1IE;

	// Set NVIC
	NVIC_SetPriority(TIM3_IRQn, 0);
	NVIC_EnableIRQ(TIM3_IRQn);
}

void TIM3_IRQHandler(void)
{
	// Update Interrupt
	if(TIM3->SR & TIM_SR_UIF)
	{
		TIM3->SR &= ~(TIM_SR_UIF);

		// Update Event
	}

	// Capture/Compare Interrupt
	if(TIM3->SR & TIM_SR_CC1IF)
	{
		TIM3->SR &= ~(TIM_SR_CC1IF);

		// Capture/Compare Event
	}
}

void LED_PwmPinSetup(void)
{
	// PA6 as PWM TIM3 CH1
	// Enable GPIOA
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

	// Alternate Function for PA6
	GPIOA->MODER &= ~(GPIO_MODER_MODE6_0);

	// Which alternate function select for this pin (Datasheet)
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL6_0;

}

void TIM3_PwmConfiguration(void)
{
	// PWM on Channel 1
	TIM3->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;

	// Compare Register
	TIM3->CCR1 = 80;

	// CC Enable
	TIM3->CCER |= TIM_CCER_CC1E;
}

void LD4_Configure(void)
{
	// Enable Clock for PORTD
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

	// Configure GPIO Mode - Output
	GPIOA->MODER |= GPIO_MODER_MODE5_0; // It's default reset state. Not necessary.
	GPIOA->MODER &= ~(GPIO_MODER_MODE5_1);

	// Configure Output Mode - Push-pull
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT5); // It's default reset state. Not necessary.

	// Configure GPIO Speed - Low
	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED5); // Two bits together. It's default reset state. Not necessary.

	// Configure Pull-up/Pull-down - no PU/PD
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD5); // It's default reset state. Not necessary.
}

void SysTick_Handler(void)
{
	Tick++; // Increase system timer
}

uint32_t GetSystemTick(void)
{
	return Tick; // Return current System Time
}

