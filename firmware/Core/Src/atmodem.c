/*
 * atmodem.c
 *
 *  Created on: Mar 6, 2024
 *      Author: cepa
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis_os.h"
#include "atmodem.h"
#include "debug.h"
#include "printf.h"

/*
 * Try to avoid overrun lock issue
 * https://github.com/ARMmbed/mbed-os/issues/14925
 */
static void __uartOverrunFix(ATModem_t *atm)
{
	if (__HAL_UART_GET_FLAG(atm->huart, UART_FLAG_ORE)) {
		__HAL_UART_CLEAR_OREFLAG(atm->huart);
	}
}

void ATModem_Init(ATModem_t *atm, UART_HandleTypeDef *huart)
{
	memset(atm, 0, sizeof(ATModem_t));
	atm->huart = huart;
}

void ATModem_UART_Callback(ATModem_t *atm)
{
	if (atm->uartRxIndex > ATMODEM_BUF_SIZE - 1) {
		atm->uartRxIndex = 0;
	}

	// Append byte
	atm->uartRxBuf[atm->uartRxIndex++] = atm->uartRxByte;
	atm->uartRxBuf[atm->uartRxIndex] = '\0';



	// Listen for next byte
	__uartOverrunFix(atm);
	HAL_UART_Receive_IT(atm->huart, &atm->uartRxByte, 1);
}

HAL_StatusTypeDef ATModem_SendCommand(ATModem_t *atm, const char *fmt, ...)
{
	va_list args;
	char command[128];
	PrintfLock();
	va_start(args, fmt);
	vsnprintf(command, 128, fmt, args);
	va_end(args);
	PrintfUnlock();
	_DEBUG(command);

	// Reset RX buffer
	atm->uartRxIndex = 0;
	atm->uartRxBuf[0] = '\0';

	return HAL_UART_Transmit(atm->huart, (uint8_t *)command, strlen(command), 100);
}

HAL_StatusTypeDef ATModem_SendPayload(ATModem_t *atm, const char *payload)
{
	// Reset RX buffer
	atm->uartRxIndex = 0;
	atm->uartRxBuf[0] = '\0';

	_DEBUG(payload);

	return HAL_UART_Transmit(atm->huart, (uint8_t *)payload, strlen(payload), 100);
}


const char *ATModem_WaitForReply(ATModem_t *atm, int msTimeout)
{
	return ATModem_WaitForExpectedReply(atm, (const char *[]){"OK", "ERROR"}, 2, msTimeout);
}

const char *ATModem_WaitForExpectedReply(ATModem_t *atm, const char *expected[], int nExpected, int msTimeout)
{
	const char *buf = (const char *)atm->uartRxBuf;

	// Enable RX interrupt, receive next byte
	__uartOverrunFix(atm);
	HAL_UART_Receive_IT(atm->huart, &atm->uartRxByte, 1);

	do {
		for (int i = 0; i < nExpected; i++) {
			if (strstr(buf, expected[i]) != NULL) {
				_DEBUG("%s\r\n", buf);
				return buf;
			}
		}

		// Wait 100ms
		osDelay(100);
		msTimeout -= 100;
	} while (msTimeout > 0);

	return NULL;
}
