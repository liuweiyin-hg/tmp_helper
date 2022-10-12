/*
 * ds1302.c
 *
 *  Created on: 2022年8月19日
 *      Author: liuweiyin
 */
#include <string.h>
#include <stdio.h>
#include "ds1302.h"

// Write Register Address
#define DS1302_SEC				0x80
#define DS1302_MIN				0x82
#define DS1302_HOUR				0x84
#define DS1302_DATE				0x86
#define DS1302_MONTH			0x88
#define DS1302_DAY				0x8a
#define DS1302_YEAR				0x8c
#define DS1302_CONTROL		0x8e
#define DS1302_CHARGER		0x90
#define DS1302_CLKBURST		0xbe
#define DS1302_RAMBURST 	0xfe

#define HEX2BCD(v)	((v) % 10 + (v) / 10 * 16)
#define BCD2HEX(v)	((v) % 16 + (v) / 16 * 10)

// SDA Read(input) Mode
static void readSDA(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.Pin = DS1302_SDA_Pin;
	GPIO_InitStructure.Mode =  GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(DS1302_SDA_GPIO_Port, &GPIO_InitStructure);
}

// SDA Write(output) Mode
static void writeSDA(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.Pin = DS1302_SDA_Pin;
	GPIO_InitStructure.Mode =  GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(DS1302_SDA_GPIO_Port, &GPIO_InitStructure);

}

static void delayUS_DWT(uint16_t us) {
	__HAL_TIM_SET_COUNTER(&TIM_HANDLE, 0);
	while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

/* Sends an address or command */
static void DS1302_SendCmd(uint8_t cmd) {
	uint8_t i;
	for (i = 0; i < 8; i ++)
	{
//		DS1302_SDA = (bit)(addr & 1);
		HAL_GPIO_WritePin(DS1302_SDA_GPIO_Port, DS1302_SDA_Pin, (cmd & 1) ?  GPIO_PIN_SET :  GPIO_PIN_RESET);
//		DS1302_SCK = 1;
		HAL_GPIO_WritePin(DS1302_SDA_GPIO_Port, DS1302_SCLK_Pin,  GPIO_PIN_SET);
		delayUS_DWT(10);
//		DS1302_SCK = 0;
		HAL_GPIO_WritePin(DS1302_SDA_GPIO_Port, DS1302_SCLK_Pin,  GPIO_PIN_RESET);
		delayUS_DWT(10);
		cmd >>= 1;
	}
}

/* Writes a byte to 'addr' */
static void DS1302_WriteByte(uint8_t addr, uint8_t d)
{
	uint8_t i;

//	DS1302_RST = 1;
	HAL_GPIO_WritePin(DS1302_RST_GPIO_Port, DS1302_RST_Pin,  GPIO_PIN_SET);

	//addr = addr & 0xFE;
	DS1302_SendCmd(addr);	// Sends address

	for (i = 0; i < 8; i ++)
	{
//		DS1302_SDA = (bit)(d & 1);
		HAL_GPIO_WritePin(DS1302_SDA_GPIO_Port, DS1302_SDA_Pin, (d & 1) ?  GPIO_PIN_SET :  GPIO_PIN_RESET);
		delayUS_DWT(1);
//		DS1302_SCK = 1;
		HAL_GPIO_WritePin(DS1302_SCLK_GPIO_Port, DS1302_SCLK_Pin,  GPIO_PIN_SET);
		delayUS_DWT(10);
//		DS1302_SCK = 0;
		HAL_GPIO_WritePin(DS1302_SCLK_GPIO_Port, DS1302_SCLK_Pin,  GPIO_PIN_RESET);
		delayUS_DWT(10);
		d >>= 1;
	}

//	DS1302_RST = 0;
	HAL_GPIO_WritePin(DS1302_RST_GPIO_Port, DS1302_RST_Pin,  GPIO_PIN_RESET);
	//	DS1302_SDA = 0;
	HAL_GPIO_WritePin(DS1302_SDA_GPIO_Port, DS1302_SDA_Pin,  GPIO_PIN_RESET);
}

/* Reads a byte from addr */
static uint8_t DS1302_ReadByte(uint8_t addr)
{
	uint8_t i;
	uint8_t temp = 0;
	uint8_t buf[8];
	uint8_t chk[10];
	for (i=0; i<10; i++) {
		chk[i] = 0;
	}

//	DS1302_RST = 1;
	HAL_GPIO_WritePin(DS1302_RST_GPIO_Port, DS1302_RST_Pin,  GPIO_PIN_SET);
	addr = addr | 0x01; // Generate Read Address

	DS1302_SendCmd(addr);	// Sends address

	readSDA();
	for (i = 0; i < 8; i ++)
	{

//		if(DS1302_SDA)
		if(HAL_GPIO_ReadPin(DS1302_SDA_GPIO_Port, DS1302_SDA_Pin)) {
			temp |= 0x80;
			chk[i] = 1;
		} else {
			chk[i] = 0;
		}
//		DS1302_SCK = 1;
		HAL_GPIO_WritePin(DS1302_SCLK_GPIO_Port, DS1302_SCLK_Pin,  GPIO_PIN_SET);
		delayUS_DWT(10);
//		DS1302_SCK = 0;
		HAL_GPIO_WritePin(DS1302_SCLK_GPIO_Port, DS1302_SCLK_Pin,  GPIO_PIN_RESET);



		delayUS_DWT(10);

		if (i<7)
			temp = temp >> 1;
	}
	writeSDA();

//	DS1302_RST = 0;
	HAL_GPIO_WritePin(DS1302_RST_GPIO_Port, DS1302_RST_Pin,  GPIO_PIN_RESET);
//	DS1302_SDA = 0;
	HAL_GPIO_WritePin(DS1302_SDA_GPIO_Port, DS1302_SDA_Pin,  GPIO_PIN_RESET);

//	for (i=0; i<10; i++) {
//		sprintf((char*)buf, "k:%d\r\n", chk[i]);
//		HAL_UART_Transmit(&huart2, buf, strlen((char*)buf), HAL_MAX_DELAY);
//	}
	return temp;
}

/* Initialization */
void DS1302_Init(void)
{
//	DS1302_WriteByte(DS1302_CHARGER, 0x5c);
	HAL_GPIO_WritePin(DS1302_SCLK_GPIO_Port, DS1302_SCLK_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DS1302_RST_GPIO_Port, DS1302_RST_Pin, GPIO_PIN_RESET);
}

/* Reads time byte by byte to 'buf' */
void DS1302_ReadTime(uint8_t *datetime)
{
   	uint8_t tmp;
//   	uint8_t buf_1[16];

   	/*
   	tmp = DS1302_ReadByte(DS1302_YEAR);
	sprintf((char*)buf_1, "Y:%x,\r\n", tmp);
	HAL_UART_Transmit(&UART_HANDLE, buf_1, strlen((char*)buf), HAL_MAX_DELAY);

	tmp = DS1302_ReadByte(DS1302_MONTH);
	sprintf((char*)buf_1, "M:%x,\r\n", tmp);
	HAL_UART_Transmit(&UART_HANDLE, buf_1, strlen((char*)buf), HAL_MAX_DELAY);

	tmp = DS1302_ReadByte(DS1302_DATE);
	sprintf((char*)buf_1, "d:%x,\r\n", tmp);
	HAL_UART_Transmit(&UART_HANDLE, buf_1, strlen((char*)buf), HAL_MAX_DELAY);

	tmp = DS1302_ReadByte(DS1302_DAY);
	sprintf((char*)buf_1, "D:%x,\r\n", tmp);
	HAL_UART_Transmit(&UART_HANDLE, buf_1, strlen((char*)buf), HAL_MAX_DELAY);
   	*/
   	tmp = DS1302_ReadByte(DS1302_HOUR);
//	sprintf((char*)buf_1, "H:%x,\r\n", tmp);
//	HAL_UART_Transmit(&UART_HANDLE, buf_1, strlen((char*)buf_1), HAL_MAX_DELAY);
	datetime[4] = BCD2HEX(tmp);

   	tmp = DS1302_ReadByte(DS1302_MIN);
//	sprintf((char*)buf_1, "i:%x\r\n", tmp);
//	HAL_UART_Transmit(&UART_HANDLE, buf_1, strlen((char*)buf_1), HAL_MAX_DELAY);
	datetime[5] = BCD2HEX(tmp);

	tmp = DS1302_ReadByte(DS1302_SEC);
	tmp = tmp & 0x7F;
//	sprintf((char*)buf_1, "S:%x,\r\n", tmp);
//	HAL_UART_Transmit(&UART_HANDLE, buf_1, strlen((char*)buf_1), HAL_MAX_DELAY);
	datetime[6] = BCD2HEX(tmp);
}

/* Writes time byte by byte from 'buf' */
void DS1302_WriteTime(uint8_t *buf)
{
	DS1302_WriteByte(DS1302_CONTROL,0x00);			// Disable write protection
	delayUS_DWT(1);
	DS1302_WriteByte(DS1302_SEC,0x30);
	DS1302_WriteByte(DS1302_MIN, 0x09);
	DS1302_WriteByte(DS1302_HOUR, 0x18);
	DS1302_WriteByte(DS1302_DATE, 0x08);
	DS1302_WriteByte(DS1302_MONTH,0x08);
	DS1302_WriteByte(DS1302_YEAR, 0x08);
//	DS1302_WriteByte(DS1302_MONTH,0x01);
//	DS1302_WriteByte(DS1302_DATE,HEX2BCD(buf[3]));
//	DS1302_WriteByte(DS1302_HOUR,HEX2BCD(buf[4]));
//	DS1302_WriteByte(DS1302_MIN,HEX2BCD(buf[5]));
//	DS1302_WriteByte(DS1302_SEC,HEX2BCD(buf[6]));
//	DS1302_WriteByte(DS1302_DAY,HEX2BCD(buf[7]));
	DS1302_WriteByte(DS1302_CONTROL,0x80);			// Enable write protection
	delayUS_DWT(1);
}

void DS1302_TestTim() {
	while(1) {
		HAL_GPIO_TogglePin(DS1302_SCLK_GPIO_Port, DS1302_SCLK_Pin);
		delayUS_DWT(10);
	}
}
