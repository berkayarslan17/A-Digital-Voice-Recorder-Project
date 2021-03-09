/*
 * main.c
 *
 *  	Created on: 14 Jan 2020
 *  	Author: Berkay Arslan / 171024030
 */

#include "stm32g0xx.h"
#include "ssdfunctions.h"
#include "keypadfunctions.h"

		// Global Variables

//Keypad Variables
_Bool prevent_bounce = 0;

//SSD Variables
int First_Digit_Value;
int Second_Digit_Value;
int Third_Digit_Value;
int Fourth_Digit_Value;
int seconds = 0;
float number = 0;


//EEPROM Variables
uint8_t EEPROM1_ADDR = 0x50;
uint8_t Audio_Data[32];
uint8_t Recorded_Audio_Data[32];
uint8_t Audio_Value;
uint8_t frequency;
uint16_t TRACK_ADDR = 0x0;
int element=0;
static volatile int buf1 = 0;
static volatile int buf2 = 0;

// Flags
_Bool Record_Flag = 0;
_Bool Read_Flag = 0;
_Bool Start_Flag = 1; //Open the Flag
_Bool Delete_Flag = 0;

// Letters
int I = 1;
int R = 10;
int C = 11;
int D = 12;
int P = 13;
int L = 14;
int B = 15;
int E = 16;


// Declare Functions
void Setup_I2C (void);
void start_conversion(void);
void Setup_ADC(void);
void TIM3_IRQHandler(void);
void EXTI0_1_IRQHandler(void);
void EXTI4_15_IRQHandler(void);
void Setup_TIM1(void);
void Setup_TIM2(void);
void Setup_TIM3(void);
void Setup_GPIO(void);
void sysclock_64M(void);

int main(void)
{
	//sysclock_64M();

	Setup_I2C();

    Setup_ADC();

    Setup_TIM3();

	Setup_GPIO();

	Setup_TIM2();

	Setup_TIM1();

	Setup_TIM16();

	Setup_TIM17();

    start_conversion();

    while(1){}

    return 0;
}

void sysclock_64M()
{
	RCC->CR |= (1U << 8);				// Enable HSI16
	while(!(RCC->CR & (1U << 10)));		// Wait for crystal

	FLASH->ACR |= (1U << 8);			// CPU Prefetch enable
	FLASH->ACR &= ~(1U << 0);			// Flash memory access latency; reset
	FLASH->ACR &= ~(1U << 1);			// Flash memory access latency; reset
	FLASH->ACR &= ~(1U << 2);			// Flash memory access latency; reset
	FLASH->ACR |= (1U << 0);			// Flash memory access latency; set to 1 wait state

	RCC->PLLCFGR |= (2U << 0);			// Select HSI16 for pll source
	RCC->PLLCFGR |= (0U << 4);			// PLLM division /1
	RCC->PLLCFGR &= ~(0x7FU << 8);		// PLL frequency multiplication factor N; reset
	RCC->PLLCFGR |= (0x8U << 8);		// PLL frequency multiplication factor N; x8
	RCC->PLLCFGR &= ~(1U << 29);		// PLL VCO division factor R for PLLRCLK clock output; reset
	RCC->PLLCFGR &= ~(1U << 30);		// PLL VCO division factor R for PLLRCLK clock output; reset
	RCC->PLLCFGR &= ~(1U << 31);		// PLL VCO division factor R for PLLRCLK clock output; reset
	RCC->PLLCFGR |= (1U << 29);			// PLL VCO division factor R for PLLRCLK clock output; /2
    RCC->PLLCFGR |= (1U << 28);			// PLLRCLK clock output enable

	RCC->CFGR &= ~(1U << 11);			// AHB prescaler to /1
    RCC->CFGR &= ~(1U << 14);			// APB prescaler to /1

    RCC->CR |= (1U << 24);				// PLLON: PLL enable
    while(!(RCC->CR & (1U << 25)));		// Wait for PLL to stable

    RCC->CFGR &= ~(1U << 0);			// System clock switch to; reset
    RCC->CFGR &= ~(1U << 1);			// System clock switch to; reset
    RCC->CFGR |= (2U << 0);			// System clock switch to; PLLRCLK

    while(!(RCC->CFGR & (2U << 3)));	// Wait for PLL to stable

    SystemCoreClockUpdate();

    SysTick_Config(SystemCoreClock);

}

//Transition between digits
void display_number(int value_num, int digit_num)
{
	if(digit_num == 1) {Digit_All_Off(); Digit_One_ON();}

	else if(digit_num == 2) {Digit_All_Off(); Digit_Two_ON();}

	else if(digit_num == 3) {Digit_All_Off(); Digit_Three_ON();}

	else {Digit_All_Off(); Digit_Four_ON();}

	setSSD(value_num);
}

void Display_SSD(void)
{
	static int move_it = 0; // i like to :)

	switch(move_it)
	{
	case 0:
		display_number(First_Digit_Value, 1);
		move_it++;
		break;
	case 1:
		display_number(Second_Digit_Value, 2);
		move_it++;
		break;
	case 2:
		display_number(Third_Digit_Value, 3);
		move_it++;
		break;
	case 3:
		display_number(Fourth_Digit_Value, 4);
		move_it = 0;
		break;
	}
}

// There are five states (Start, IDLE, Record, Playback and Clear)
void Change_State(int num1, int num2, int num3, int num4)
{
	First_Digit_Value  = num1;
	Second_Digit_Value = num2;
	Third_Digit_Value  = num3;
	Fourth_Digit_Value = num4;
}

void random_read_I2C(uint8_t devAddr, uint16_t memAddr, uint8_t* data, int size)
{
	I2C1->CR2 = 0;
	I2C1->CR2 |= (uint32_t)(devAddr << 1);
	I2C1->CR2 |= (2U << 16);	//Number of bytes
	I2C1->CR2 |= (1U << 13);	//Generate Start

	while(!(I2C1->ISR & (1 << 1)));
	I2C1->TXDR = (uint32_t)(memAddr >> 8);

	while(!(I2C1->ISR & (1 << 1)));
	I2C1->TXDR = (uint32_t)(memAddr & 0xFF);

	while(!(I2C1->ISR & (1 << 6)));	//until the transmission complete

	I2C1->CR2 = 0;
	I2C1->CR2 |= (uint32_t)(devAddr << 1);
	I2C1->CR2 |= (1U << 10);	//Read mode
	I2C1->CR2 |= (uint32_t)(size << 16);	//Number of bytes
	I2C1->CR2 |= (1U << 25);	//AUTOEND
	I2C1->CR2 |= (1U << 13);	//Generate start

	for(int i = 0;  i < size; i++)
	{
		while(!(I2C1->ISR & (1 << 2)));
		data[i++] = (uint8_t)I2C1->RXDR;
	}

}

void write_memory_I2C(uint8_t devAddr, uint16_t memAddr,uint8_t *data, int size)
{
	I2C1->CR2 = 0;
	I2C1->CR2 |= (uint32_t)(devAddr << 1);
	I2C1->CR2 |= (uint32_t)((size + 2)<< 16);
	I2C1->CR2 |= (1U << 25);	//Autoend
	I2C1->CR2 |= (1U << 13);	//Generate start

	while(!(I2C1->ISR & (1 << 1)));	// high address byte
	I2C1->TXDR = (uint32_t)(memAddr >> 8);

	while(!(I2C1->ISR & (1 << 1)));	// low address byte
	I2C1->TXDR = (uint32_t)(memAddr & 0xFF);

	for(int i = 0;  i < size; i++)
	{
		while(!(I2C1->ISR & (1 << 1)));
		I2C1->TXDR = (*data++);	//send data
	}
}

void start_conversion(void)
{
    ADC1->CR |= (1U << 2);
    while(!(ADC1->ISR & (1U << 2)));
}

void Setup_ADC(void)
{
	RCC->IOPENR |= (1U << 0);
	GPIOA->MODER &= ~(3U << 2 * 0);		/* Clear the MODER for PA0 */
	GPIOA->MODER |= (3U << 2 * 0);		/* Set PA0 to Analog mode */

	RCC->APBENR2 |= (1U << 20);			/* Enable ADC clock */
	ADC1->CR |= (1U << 28);	 			/* Enable the regulator */
	delay(100);							/* Wait until it is initialized  */

	ADC1->CR |= (1U << 31);				/* Enable ADC Calibration  */
	while (!(ADC1->ISR & (1 << 11)));	/* Wait until it is initialized */

	ADC1->IER |= (1U << 2);				// end of conversion interrupt enable

	ADC1->CFGR1 |= (2U << 3);		// 8-bit resolution

	ADC1->CFGR1 &= ~(1U << 13);		// Single conversion mode
	ADC1->CFGR1 |=  (1U << 16);		// Discontinuous mode

	//TIM3 TRGO
	ADC1->CFGR1 |= (3U << 6);		// TIM3_TRGO is 011 in EXTSEL
	ADC1->CFGR1 |= (1U << 10);		// Hardware trigger detection on the rising edge in EXTEN

	ADC1->CHSELR |= (1U << 0);		// ADC channel selection (PA0)
	ADC1->CFGR1 |= (1U << 23);
	ADC1->CFGR1 |= (0U << 26);

	ADC1->CR |= (1U << 0);				/* Enable ADC */
	while (!(ADC1->ISR & (1 << 0)));	/* Until ADC ready */

	NVIC_SetPriority(ADC1_IRQn, 1);
	NVIC_EnableIRQ(ADC1_IRQn);
}

void Setup_I2C (void)
{
	    RCC->IOPENR |= (1U << 1);

	    GPIOB->MODER &= ~(3U << 2 * 6);
	    GPIOB->MODER |= (2U << 2 * 6);
	    GPIOB->PUPDR |= (1U << 2 * 6);

	    GPIOB->MODER &= ~(3U << 2 * 7);
	    GPIOB->MODER |= (2U << 2 * 7);
	    GPIOB->PUPDR |= (1U << 2 * 7);

	    GPIOB->OTYPER &= ~(1U << 7);
	    GPIOB->OTYPER &= ~(1U << 6);

	    GPIOB->OTYPER |= (1U << 6);
	    GPIOB->OTYPER |= (1U << 7);

	    GPIOB->AFR[0] &= ~(0xFU << 4 * 6);
	    GPIOB->AFR[0] |= (6U << 4 * 6);

	    GPIOB->AFR[0] &= ~(0xFU << 4 * 7);
	    GPIOB->AFR[0] |= (6U << 4 * 7);

	    //Enable I2C

	    RCC->APBENR1 |= (1U << 21);
	    I2C1->CR1 = 0;
	    I2C1->CR1 |= (1U << 7);

	    I2C1->TIMINGR |= (3 << 2 * 8);
	    I2C1->TIMINGR |= (0x13 << 0);
	    I2C1->TIMINGR |= (0xF << 8);
	    I2C1->TIMINGR |= (0x2 << 1 * 6);
	    I2C1->TIMINGR |= (0x4 << 2 * 0);

	    I2C1->CR1 |= (1U << 0);

	    NVIC_SetPriority(I2C1_IRQn,1);
	    NVIC_EnableIRQ(I2C1_IRQn);
}

void Setup_TIM1(void)
{
	/* Setup TIM1 */
	RCC->APBENR2 |= (1U << 11);
	TIM1->CR1 = 0;
	TIM1->CR1 |= (1 << 7);
	TIM1->CNT = 0;
	TIM1->PSC = 0;
	TIM1->ARR = 2000;
	TIM1->DIER |= (1 << 0);
	TIM1->CR1 |= (1 << 0);
	NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 1);
	NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
}

void Setup_TIM2(void)
{
	RCC->APBENR1 |= (1U << 0);

	/* Setup PWM */
	RCC->IOPENR |= (1U << 1);
	GPIOB->MODER &= ~(3U << 2 * 3);
	GPIOB->MODER |= (2U << 2 * 3);
	GPIOB->AFR[0] &= ~(0xFU << 4 * 3);	//Clear the bits for AFSEL3
	GPIOB->AFR[0] |= (2U << 4 * 3);		// AF2 0010 for AFSEL3

	/* Setup CH2 */
	TIM2->CCMR1 &= ~(0X7U << 12);
	TIM2->CCMR1 &= ~(0X1U << 24);
	TIM2->CCMR1 |= (0X6U << 12);  	/* OC2M: -0110- PWM mode 1 */
	TIM2->CCMR1 |= (1U << 11); 		/* OC2FE: Enable output compare 2 */
	TIM2->CCER |= (1U << 4);
	TIM2->CR1 |= (1U << 7);

	TIM2->DIER |= (1U << 0);

	TIM2->CNT = 0;

	/*
	 * The formula: PWM freq = Fclk / (PSC * ARR) - Fclk = 16Mhz
	 * 				PWM duty =CCR2 / ARR
	 */
	//TIM2->PSC = 0;
	TIM2->ARR = 255;	// almost 65khz

	TIM2->EGR |= (1 << 0); 			/* UPDATE GENERATION */

	TIM2->CR1 |= (1U << 0);

}

void Setup_TIM3(void)
{
	/* Setup TIM3 */
	RCC->APBENR1 |= (1U << 1);

	TIM3->CR1 = 0;
	TIM3->CR1 |= (1U << 7);
	TIM3->CNT = 0;

	TIM3->CR2 |= (2U << 4);		//MMS REGISTER UPDATE MODE

	TIM3->PSC = 0;
	TIM3->ARR = 2000;	//8khz

	TIM3->DIER |= (1U << 0);
	TIM3->CR1 |= (1U << 0);
}

void Setup_TIM16(void)
{
	RCC->APBENR2 |= (1U << 17);

	TIM16->CR1 = 0;
	TIM16->CR1 |= (1U << 7);

	TIM16->CNT = 0;
	TIM16->PSC = 9999;
	TIM16->ARR = 6400;

	TIM16->DIER |= (1U << 0);
	TIM16->CR1 |= (1U << 0);

	NVIC_SetPriority(TIM16_IRQn, 2);
	NVIC_EnableIRQ(TIM16_IRQn);
}

void Setup_TIM17(void)
{
	RCC->APBENR2 |= (1U << 18);

	TIM17->CR1 = 0;
	TIM17->CR1 |= (1U << 7);

	TIM17->CNT = 0;
	TIM17->PSC = 0;
	TIM17->ARR = 1000;

	TIM17->DIER |= (1U << 0);
	TIM17->CR1 |= (1U << 0);

	NVIC_SetPriority(TIM17_IRQn, 2);
	NVIC_EnableIRQ(TIM17_IRQn);
}

void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
	if (Read_Flag == 1)
	{
		if(TRACK_ADDR == 0xFA00)
		{
			Read_Flag = 0;
			TRACK_ADDR = 0;
			element = 0;
		}

		if(element == 32)
		{
			random_read_I2C(EEPROM1_ADDR, TRACK_ADDR, &Audio_Data, 32);
			delay(1000);
			TRACK_ADDR += 32;
			element = 0;
			TIM2->CCR2 = Audio_Data[element];
		}

		else if(element < 32)
		{
			TIM2->CCR2 = Audio_Data[element];
			element++;
		}
	}
	TIM1->SR &= ~(1U << 0);
}

void ADC_COMP_IRQHandler(void)
{
	//Write to eeprom for five seconds
	if (Record_Flag == 1)
	{

		if(TRACK_ADDR == 0xFA00)	// If the EEPROM's memory is full.
		{
			Record_Flag = 0;
			TRACK_ADDR = 0;
			element = 0;
		}

		Audio_Value = (uint8_t) (ADC1->DR);
		Audio_Data[element] = Audio_Value;
		element++;
		if(element == 32)
		{
			write_memory_I2C(EEPROM1_ADDR, TRACK_ADDR, &Audio_Data, 32);
			element = 0;
			TRACK_ADDR += 32;
		}
	}
//	//Read from eeprom for five seconds
//	if(Read_Flag == 1)
//	{
//	    TIM2->CCR2 = Audio_Value;
//	    Audio_Value = ((uint8_t)Recorded_Audio_Data[element]);
//		element++;
//
//		random_read_I2C(EEPROM1_ADDR, TRACK_ADDR, (uint8_t *)&Recorded_Audio_Data, 1);
//		TRACK_ADDR += 1;
//		element = 0;
//
//		if(seconds == 0)
//		{
//			element = 0;
//			Read_Flag = 0;
//			TRACK_ADDR = 0;
//		}
//	}
	//Clear the bytes in eeprom's memory.
	if (Delete_Flag == 1)
	{
		if(TRACK_ADDR == 0xFA00)	// If the the TRACK_ADDR reached its full value.
		{
			Delete_Flag = 0;
			TRACK_ADDR = 0;
			element = 0;
		}

		Audio_Data[element] = 0;
		Recorded_Audio_Data[element] = 0;
		element++;
		if(element == 32)
		{
			write_memory_I2C(EEPROM1_ADDR, TRACK_ADDR, &Audio_Data, 32);
			TRACK_ADDR += 32;
			element = 0;
		}
	}
		ADC1->ISR &= (1U << 2); /* Clear Interrupt */
}

void TIM16_IRQHandler(void)
{
	//Count back when record flag is open
	if(Record_Flag == 1 && seconds > 0) {seconds--;}
	//Count back when read flag is open
	else if (Read_Flag == 1 && seconds > 0) {seconds--;}
	//Count back when delete flag is open
	else if (Delete_Flag == 1 && seconds > 0) {seconds--;}
	// change state to idle state
	else {Delete_Flag = 0; Record_Flag = 0; Read_Flag = 0;}


	TIM16->SR &= ~(1U << 0);
}

void TIM17_IRQHandler(void)
{
	prevent_bounce = 0;

	// Start State shows the ID number
	if(Start_Flag == 1) {Change_State(1, D, L, E);}

	// Record state shows rcd and counts down from 5 seconds indicating the recording.
	else if(Record_Flag == 1) {Change_State(R, C, D, seconds);}

	//PLAYBACK state where the 7SD shows PLb.
	else if(Read_Flag == 1) {Change_State(P, L, B, seconds);}

	//Clear state where the 7SD shows clr and clear the EEPROM.
	else if (Delete_Flag == 1) {Change_State(C, L, R, seconds);}

	// IDLE State
	else {Change_State(1, D, L, E);}

	Display_SSD();

	TIM2->SR &= ~(1U << 0);
}

void Setup_GPIO(void){

    // SSD SETUP
    RCC -> IOPENR |= (7U << 0); //Enabling GPIOA, GPIOB, GPIOC

    GPIOA -> MODER &= ~(3U << 2 * 1); // Clearing the bits for PA1
    GPIOA -> MODER |= (1U << 2 * 1);   // Setupping the bits for PA1

    GPIOA -> MODER &= ~(15U << 2 * 4); // Clearing the bits for PA4-PA5
    GPIOA -> MODER |= (5U << 2 * 4);   // Setupping the bits for PA4-PA5

    GPIOA -> MODER &= ~(15U << 2 * 6); // Clearing the bits for PA6-PA7
    GPIOA -> MODER |= (5U << 2 * 6);   // Setupping the bits for PA6-PA7

    GPIOA -> MODER &= ~(15U << 2 * 11); // Clearing the bits for PA11-PA12
    GPIOA -> MODER |= (5U << 2 * 11);   // Setupping the bits for PA11-PA12

    GPIOB -> MODER &= ~(15U << 2 * 4); // Clearing the bits for PB4-PB5
    GPIOB -> MODER |= (5U << 2 * 4);   // Setupping the bits for PB4-PB5

    GPIOB -> MODER &= ~(15U << 2 * 8); // Clearing the bits for PB8-PB9
    GPIOB -> MODER |= (5U << 2 * 8);   // Setupping the bits for PB8-PB9

    // SET KEY PAD AS AN OUTPUT -ROW

    GPIOA -> MODER &= ~(3U << 2 * 15); // Clearing the bits for R1 -PA15
    GPIOA -> MODER |= (1U << 2 * 15);   // Setupping the bits for R1

    GPIOB -> MODER &= ~(3U << 2 * 1); // Clearing the bits for R2 -PB1
    GPIOB -> MODER |= (1U << 2 * 1);   // Setupping the bits for R2

    GPIOA -> MODER &= ~(3U << 2 * 10); // Clearing the bits for R3 -PA10
    GPIOA -> MODER |= (1U << 2 * 10);   // Setupping the bits for R3

    // SET KEY PAD AS AN INPUT -COLUMN

    GPIOA -> MODER &= ~(3U << 2 * 9); // Clearing the bits for C1 -PA9
    GPIOA -> PUPDR |= (2U << 2 * 9);   // Pull-Down mode for C1

    GPIOB -> MODER &= ~(3U << 2 * 0); // Clearing the bits for C2 -PB0
    GPIOB -> PUPDR |= (2U << 2 * 0);   // Pull-Down mode for C2

    GPIOB -> MODER &= ~(3U << 2 * 2); // Clearing the bits for C3 -PB2
    GPIOB -> PUPDR |= (2U << 2 * 2);   // Pull-Down mode for C3

    // SETUP INTERRUPTS FOR INPUTS
    EXTI -> EXTICR[2] |= (0U << 8 * 1); // PA9
    EXTI -> EXTICR[0] |= (1U << 8 * 0); // PB0
    EXTI -> EXTICR[0] |= (1U << 8 * 2); // PB2

    //RISING EDGE
    EXTI -> RTSR1 |= (1U << 9);
    EXTI -> RTSR1 |= (1U << 0);
    EXTI -> RTSR1 |= (1U << 2);

    // MASK
    EXTI -> IMR1 |= (1U << 9);
    EXTI -> IMR1 |= (1U << 0);
    EXTI -> IMR1 |= (1U << 2);

    // NVIC
    NVIC_SetPriority(EXTI0_1_IRQn, 0);
    NVIC_EnableIRQ(EXTI0_1_IRQn);

    NVIC_SetPriority(EXTI2_3_IRQn, 0);
    NVIC_EnableIRQ(EXTI2_3_IRQn);

    NVIC_SetPriority(EXTI4_15_IRQn, 0);
    NVIC_EnableIRQ(EXTI4_15_IRQn);

    setRowsKeypad();

    clearSSD();
}

void EXTI0_1_IRQHandler(void)
{
    // interrupts from PB0
    clearRowsKeypad();
    GPIOA -> ODR ^= (1U << 15);  //PA15
    if((GPIOB ->IDR & (1U << 0)) && prevent_bounce == 0)
    {Record_Flag = 0; Start_Flag = 0; Read_Flag = 1; seconds = 5; prevent_bounce = 1;}
    GPIOA -> ODR ^= (1U << 15);  //PA15

    GPIOB -> ODR ^= (1U << 1);  //PB1
    if((GPIOB ->IDR & (1U << 0)) && prevent_bounce == 0)
    {number = 5; prevent_bounce = 1;}
    GPIOB -> ODR ^= (1U << 1);  //PB1

    GPIOA -> ODR ^= (1U << 10);  //PA10
    if((GPIOB ->IDR & (1U << 0)) && prevent_bounce == 0)
    {number = 8; prevent_bounce = 1;}
    GPIOA -> ODR ^= (1U << 10);  //PA10

    EXTI -> RPR1 |= (1U << 0);
    setRowsKeypad();
}

void EXTI2_3_IRQHandler(void)
{
    // interrupts from PB2
    clearRowsKeypad();
    GPIOA -> ODR ^= (1U << 15);  //PA15
    if((GPIOB ->IDR & (1U << 2)) && prevent_bounce == 0)
    {Delete_Flag = 1; Start_Flag = 0; seconds = 5; prevent_bounce = 1;}
    GPIOA -> ODR ^= (1U << 15);  //PA15

    GPIOB -> ODR ^= (1U << 1);  //PB1
    if((GPIOB ->IDR & (1U << 2)) && prevent_bounce == 0)
    {number = 6; prevent_bounce = 1;}
    GPIOB -> ODR ^= (1U << 1);  //PB1

    GPIOA -> ODR ^= (1U << 10);  //PA10
    if((GPIOB ->IDR & (1U << 2)) && prevent_bounce == 0)
    {number = 9; prevent_bounce = 1;}
    GPIOA -> ODR ^= (1U << 10);  //PA10

    EXTI -> RPR1 |= (1U << 2);
    setRowsKeypad();
}

void EXTI4_15_IRQHandler(void)
{
    // interrupts from PA9
    clearRowsKeypad();
    GPIOA -> ODR ^= (1U << 15);  //PA15
    if((GPIOA ->IDR & (1U << 9)) && prevent_bounce == 0)
    {Record_Flag = 1; Start_Flag = 0; Read_Flag = 0; seconds = 5; prevent_bounce = 1;}
    GPIOA -> ODR ^= (1U << 15);  //PA15

    GPIOB -> ODR ^= (1U << 1);  //PB1
    if((GPIOA ->IDR & (1U << 9)) && prevent_bounce == 0)
    {number = 4; Read_Flag = 1; prevent_bounce = 1;}
    GPIOB -> ODR ^= (1U << 1);  //PB1

    GPIOA -> ODR ^= (1U << 10);  //PA10
    if((GPIOA ->IDR & (1U << 9)) && prevent_bounce == 0)
    {number = 7; prevent_bounce = 1;}
    GPIOA -> ODR ^= (1U << 10);  //PA10

    EXTI -> RPR1 |= (1U << 9);
    setRowsKeypad();
}


