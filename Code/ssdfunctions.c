/*
 * ssdfunctions.c
 *
 *  Created on: 14 Jan 2020
 *      Author: Berkay Arslan / 171024030
 */

// SSD Functions

#include "stm32g0xx.h"
#include "ssdfunctions.h"



void clearSSD(void)
{
    GPIOA -> ODR |= (0xFFFFU);
}

void setSSD (int number)
{
	clearSSD();
    switch(number)
    {
        case 0:
        	GPIOA -> ODR &= ~(0x1872U);
            break;
        case 1:
            GPIOA -> ODR &= ~(0x30U);
            break;
        case 2:
            GPIOA -> ODR &= ~(0x1892U);
            break;
        case 3:
            GPIOA -> ODR &= ~(0x10B2U);
            break;
        case 4:
            GPIOA -> ODR &= ~(0xF0U);
            break;
        case 5:
            GPIOA -> ODR &= ~(0x10E2U);
            break;
        case 6:
            GPIOA -> ODR &= ~(0x18E2U);
            break;
        case 7:
            GPIOA -> ODR &= ~(0x32U);
            break;
        case 8:
            GPIOA -> ODR &= ~(0x18F2U);
            break;
        case 9:
            GPIOA -> ODR &= ~(0x10F2U);
            break;
        case 10: // r Letter
        	GPIOA -> ODR &= ~(0x880U);
        	break;
        case 11: // c Letter
        	GPIOA -> ODR &= ~(0x1880U);
        	break;
        case 12: // d Letter
        	GPIOA -> ODR &= ~(0x18B0U);
        	break;
        case 13: // P Letter
        	GPIOA -> ODR &= ~(0x8D2U);
        	break;
        case 14: // L Letter
        	GPIOA -> ODR &= ~(0x1840U);
        	break;
        case 15: // b Letter
        	GPIOA -> ODR &= ~(0x18E0U);
        	break;
        case 16: // E Letter
             GPIOA -> ODR &= ~(0x18C2U);
             break;
        default:
            break;
    }
}

void Digit_One_ON(void)
{
	GPIOB -> ODR |= (1U << 4);
}

void Digit_Two_ON(void)
{
	GPIOB -> ODR |= (1U << 5);
}

void Digit_Three_ON(void)
{
	GPIOB -> ODR |= (1U << 9);
}

void Digit_Four_ON(void)
{
	GPIOB -> ODR |= (1U << 8);
}

void Digit_One_Off(void)
{
	GPIOB -> BRR |= (1U << 4);
}

void Digit_Two_Off(void)
{
	GPIOB -> BRR |= (1U << 5);
}

void Digit_Three_Off(void)
{
	GPIOB -> BRR |= (1U << 9);
}

void Digit_Four_Off(void)
{
	GPIOB -> BRR |= (1U << 8);
}

void Digit_All_Off(void)
{
    GPIOB -> BRR |= (1U << 4);
    GPIOB -> BRR |= (1U << 5);
    GPIOB -> BRR |= (1U << 9);
    GPIOB -> BRR |= (1U << 8);
}

void delay(int delay_s)
{
	for (int i = 0; i < delay_s; ++i);
}

