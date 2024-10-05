/*
 * atmodem.h
 *
 *  Created on: Mar 6, 2024
 *      Author: cepa
 */

#ifndef INC_ATMODEM_H_
#define INC_ATMODEM_H_

#include "stm32f4xx_hal.h"

#define ATMODEM_BUF_SIZE 256

typedef struct ATModem {
	UART_HandleTypeDef *huart;
	uint8_t uartRxBuf[ATMODEM_BUF_SIZE];
	uint8_t uartRxByte;
	uint8_t uartRxIndex;
} ATModem_t;

void ATModem_Init(ATModem_t *atm, UART_HandleTypeDef *huart);
void ATModem_UART_Callback(ATModem_t *atm);
HAL_StatusTypeDef ATModem_SendCommand(ATModem_t *atm, const char *fmt, ...);
HAL_StatusTypeDef ATModem_SendPayload(ATModem_t *atm, const char *payload);
const char *ATModem_WaitForReply(ATModem_t *atm, int msTimeout);
const char *ATModem_WaitForExpectedReply(ATModem_t *atm, const char *expected[], int nExpected, int msTimeout);

#endif /* INC_ATMODEM_H_ */
