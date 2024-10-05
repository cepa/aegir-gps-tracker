/*
 * lte.h
 *
 *  Created on: Mar 18, 2024
 *      Author: cepa
 */

#ifndef INC_LTE_H_
#define INC_LTE_H_

#include "atmodem.h"

#define LTE_UART	huart2

#define LTE_OK		0
#define LTE_ERROR	1

#define LTE_NETWORK_UNREGISTERED	0
#define LTE_NETWORK_REGISTERED		1

typedef struct LTE {
	ATModem_t atm;
	int signalQuality;
	int networkStatus;
} LTE_t;

typedef uint8_t LTE_Status_t;

extern LTE_t *LTE;

LTE_Status_t LTE_Init();
LTE_Status_t LTE_Reset();
uint8_t LTE_SIMIsReady();
uint8_t LTE_NetworkIsReady();
uint8_t LTE_NetIsOpened();
uint8_t LTE_IsConnected(const char *addr, int port);
LTE_Status_t LTE_WaitForSIMReady();
LTE_Status_t LTE_WaitForNetworkReady();
LTE_Status_t LTE_UpdateSignalQuality();
LTE_Status_t LTE_NetOpen(const char *apn, const char *username, const char *password);
LTE_Status_t LTE_NetClose();
LTE_Status_t LTE_Connect(const char *addr, int port);
LTE_Status_t LTE_Close();
LTE_Status_t LTE_Send(const char *payload);
void LTE_UART_Callback();
void LTE_TaskHandler(void const *argument);

#endif /* INC_LTE_H_ */
