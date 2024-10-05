/*
 * iridium.h
 *
 *  Created on: Mar 18, 2024
 *      Author: cepa
 */

#ifndef INC_IRIDIUM_H_
#define INC_IRIDIUM_H_

#include "atmodem.h"

#define IRIDIUM_UART		huart6

#define IRIDIUM_NET_PIN		IR_NET_Pin
#define IRIDIUM_NET_PORT	IR_NET_GPIO_Port

#define IRIDIUM_OK			0
#define IRIDIUM_ERROR		1

typedef struct Iridium {
	ATModem_t atm;
	int networkAvailable;
	int signalQuality;
} Iridium_t;

typedef uint8_t Iridium_Status_t;

extern Iridium_t *Iridium;

Iridium_Status_t Iridium_Init();
Iridium_Status_t Iridium_UpdateSignalQuality();
Iridium_Status_t Iridium_Write(const char *payload);
Iridium_Status_t Iridium_Transfer();
void Iridium_UART_Callback();
void Iridium_TaskHandler(void const *argument);

#endif /* INC_IRIDIUM_H_ */
