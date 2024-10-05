/*
 * gps.h
 *
 *  Created on: Mar 18, 2024
 *      Author: cepa
 */

#ifndef INC_GPS_H_
#define INC_GPS_H_

#include "stm32f4xx_hal.h"
#include "minmea.h"

#define GPS_UART		huart1
#define GPS_BUF_SIZE	90

typedef struct minmea_float GPS_float_t;
typedef struct minmea_date GPS_date_t;
typedef struct minmea_time GPS_time_t;

typedef struct GPS {
	UART_HandleTypeDef *huart;
	uint8_t uartRxBuf[GPS_BUF_SIZE];
	uint8_t uartRxByte;
	uint8_t uartRxIndex;
	GPS_float_t latitude;
	GPS_float_t longitude;
	GPS_float_t speed;
	GPS_float_t course;
	GPS_float_t altitude;
	GPS_date_t date;
	GPS_time_t time;
	int satellites;
	int quality;
} GPS_t;

extern volatile GPS_t *GPS;

void GPS_Init();
void GPS_ParseNMEA(const char *nmea);
void GPS_UART_Callback();
void GPS_TaskHandler(void const *argument);

#endif /* INC_GPS_H_ */
