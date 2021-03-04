/*
 * ssdfunctions.h
 *
 *  Created on: 14 Jan 2020
 *      Author: Berkay Arslan / 171024030
 */

#ifndef SSDFUNCTIONS_H_
#define SSDFUNCTIONS_H_

#include "stm32g0xx.h"

void clearSSD();
void setSSD(int);

void Digit_One_ON();
void Digit_Two_ON();
void Digit_Three_ON();
void Digit_Four_ON();

void Digit_One_Off();
void Digit_Two_Off();
void Digit_Three_Off();
void Digit_Four_Off();
void Digit_All_Off();

void delay(int);

#endif /* SSDFUNCTIONS_H_ */
