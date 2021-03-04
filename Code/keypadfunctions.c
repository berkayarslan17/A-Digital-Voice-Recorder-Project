/*
 * keypadfunctions.c
 *
 *  Created on: 14 Jan 2020
 *      Author: Berkay Arslan / 171024030
 */

// Key Pad Functions

#include "stm32g0xx.h"
#include "keypadfunctions.h"

void clearRowsKeypad(void)
{
    GPIOA -> ODR &= ~(1U << 15);
    GPIOB -> ODR &= ~(1U << 1);
    GPIOA -> ODR &= ~(1U << 10);
}

void setRowsKeypad(void)
{
    GPIOA -> ODR |= (1U << 15);
    GPIOB -> ODR |= (1U << 1);
    GPIOA -> ODR |= (1U << 10);
}

