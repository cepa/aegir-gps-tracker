/*
 * gps.c
 *
 *  Created on: Mar 18, 2024
 *      Author: cepa
 */

#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "gps.h"
#include "debug.h"

static volatile GPS_t __context;
volatile GPS_t *GPS = &__context;
extern UART_HandleTypeDef GPS_UART;

void GPS_Init()
{
	memset(GPS, 0, sizeof(GPS_t));
	// Init UART
	GPS->huart = &GPS_UART;
	HAL_UART_Receive_IT(GPS->huart, &GPS->uartRxByte, 1);
}

void GPS_ParseNMEA(const char *nmea)
{
	switch (minmea_sentence_id(nmea, false)) {
	case MINMEA_SENTENCE_RMC: {
		struct minmea_sentence_rmc frame;
		if (minmea_parse_rmc(&frame, nmea)) {
			GPS->speed = frame.speed;
			GPS->course = frame.course;
			GPS->date = frame.date;
		}
	} break;

	case MINMEA_SENTENCE_GGA: {
		struct minmea_sentence_gga frame;
		if (minmea_parse_gga(&frame, nmea)) {
			GPS->latitude = frame.latitude;
			GPS->longitude = frame.longitude;
			GPS->altitude = frame.altitude;
			GPS->quality = frame.fix_quality;
			GPS->satellites = frame.satellites_tracked;
			GPS->time = frame.time;
		}
	} break;

	default:
		break;
	}
}

void GPS_UART_Callback()
{
	if (GPS->uartRxIndex > GPS_BUF_SIZE - 1) {
		GPS->uartRxIndex = 0;
	}

	// Append byte
	GPS->uartRxBuf[GPS->uartRxIndex++] = GPS->uartRxByte;
	GPS->uartRxBuf[GPS->uartRxIndex] = '\0';

	if (GPS->uartRxByte == '\n') {
		GPS_ParseNMEA((const char *)GPS->uartRxBuf);
		GPS->uartRxIndex = 0;
	}

	// Listen for next byte
	HAL_UART_Receive_IT(GPS->huart, &GPS->uartRxByte, 1);
}

void GPS_TaskHandler(void const *argument)
{
	GPS_Init();
	for (;;) {
		_DEBUG("lat=%0.6f lon=%0.6f spd=%0.2f crs=%0.2f alt=%0.2f",
				minmea_tocoord(&GPS->latitude),
				minmea_tocoord(&GPS->longitude),
				minmea_tofloat(&GPS->speed),
				minmea_tofloat(&GPS->course),
				minmea_tofloat(&GPS->altitude));
		osDelay(1000);
	}
}
