/*
 * ds1302.h
 *
 *  Created on: 2022年8月19日
 *      Author: liuweiyin
 */

#ifndef INC_DS1302_H_
#define INC_DS1302_H_

#include "main.h"

#define UART_HANDLE huart2
#define TIM_HANDLE htim2

extern UART_HandleTypeDef UART_HANDLE;
extern TIM_HandleTypeDef TIM_HANDLE;

void DS1302_Init(void);
void DS1302_ReadTime(uint8_t *buf);
void DS1302_WriteTime(uint8_t *buf);
void DS1302_TestTim();


#endif /* INC_DS1302_H_ */
