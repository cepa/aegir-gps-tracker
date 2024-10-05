/*
 * iridium.c
 *
 *  Created on: Mar 18, 2024
 *      Author: cepa
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "main.h"
#include "iridium.h"
#include "debug.h"
#include "report.h"

static Iridium_t __context;
Iridium_t *Iridium = &__context;
extern UART_HandleTypeDef IRIDIUM_UART;
QueueHandle_t iridiumQueue;

static int _iridiumNetworkAvailable()
{
	return HAL_GPIO_ReadPin(IRIDIUM_NET_PORT, IRIDIUM_NET_PIN) == GPIO_PIN_SET ? 1 : 0;
}

static int _iridiumWaitForNetworkAvailable()
{
	int retries = 600; // 600s = 10min

	do {
		if (_iridiumNetworkAvailable()) {
			return 1;
		}

		osDelay(1000); // Wait another 1s

	} while (retries-- > 0);

	return 0;
}

Iridium_Status_t Iridium_Init()
{
	memset(Iridium, 0, sizeof(Iridium_t));
	ATModem_Init(&Iridium->atm, &IRIDIUM_UART);

	// Init RockBlock transceiver
	ATModem_SendCommand(&Iridium->atm, "AT\r\n");
	ATModem_WaitForReply(&Iridium->atm, 3000);

	// Three line mode RX,TX,GND
	ATModem_SendCommand(&Iridium->atm, "AT&K0\r\n");
	ATModem_WaitForReply(&Iridium->atm, 3000);

	// Identification
	ATModem_SendCommand(&Iridium->atm, "AT+CGMI\r\n");
	ATModem_WaitForReply(&Iridium->atm, 3000);

	// Modem model
	ATModem_SendCommand(&Iridium->atm, "AT+CGMM\r\n");
	ATModem_WaitForReply(&Iridium->atm, 3000);

	return IRIDIUM_OK;
}

Iridium_Status_t Iridium_UpdateSignalQuality()
{
	const char *reply = NULL;
	ATModem_SendCommand(&Iridium->atm, "AT+CSQ\r\n");
	reply = ATModem_WaitForReply(&Iridium->atm, 30000);
	if (reply != NULL && (reply = strstr(reply, "+CSQ:")) != NULL) {
		sscanf(reply, "+CSQ: %d", &Iridium->signalQuality);
		_DEBUG("Iridium signal quality: %d", Iridium->signalQuality);
		return IRIDIUM_OK;
	}
	Iridium->signalQuality = 0;
	return IRIDIUM_ERROR;
}

Iridium_Status_t Iridium_Write(const char *payload)
{
	const char *reply = NULL;

	assert(payload != NULL);

	// Max 50 bytes
	if (strlen(payload) > 50) {
		return IRIDIUM_ERROR;
	}

	ATModem_SendCommand(&Iridium->atm, "AT+SBDWT=%s\r\n", payload);
	reply = ATModem_WaitForReply(&Iridium->atm, 30000);
	if (reply == NULL || strstr(reply, "OK") == NULL) {
		return IRIDIUM_ERROR;
	}

	return IRIDIUM_OK;
}

Iridium_Status_t Iridium_Transfer()
{
	const char *reply = NULL;
	int retries = 10;

	do {
		if (_iridiumWaitForNetworkAvailable()) {
			//Iridium_UpdateSignalQuality();
			ATModem_SendCommand(&Iridium->atm, "AT+SBDIX\r\n");
			reply = ATModem_WaitForExpectedReply(&Iridium->atm, (const char *[]){"OK", "ERROR", "+SBDIX: 0", "+SBDIX: 1", "+SBDIX: 2", "+SBDIX: 3", "+SBDIX: 4"}, 7, 30000);
			if (reply != NULL) {
				if (strstr(reply, "+SBDIX: 0,") != NULL
					|| strstr(reply, "+SBDIX: 1,") != NULL
					|| strstr(reply, "+SBDIX: 2,") != NULL
					|| strstr(reply, "+SBDIX: 3,") != NULL
					|| strstr(reply, "+SBDIX: 4,") != NULL) {
						_DEBUG("Iridium payload sent successfully");
						return IRIDIUM_OK;
				}
			}

		} else {
			return IRIDIUM_ERROR;
		}

		osDelay(3000);

	} while (retries-- > 0);

	return IRIDIUM_ERROR;
}

void Iridium_UART_Callback()
{
	ATModem_UART_Callback(&Iridium->atm);
}

void Iridium_TaskHandler(void const *argument)
{
	Report_t report;
	char buf[24];
	const char *payload = NULL;
	int i = 0, n = 0;

	Iridium_Init();

	for (;;) {

		if (_iridiumNetworkAvailable()) {
			Iridium->networkAvailable = 1;
			Iridium_UpdateSignalQuality();

			i = 0;
			while ((n = uxQueueMessagesWaiting(iridiumQueue)) > 0) {
				if (xQueuePeek(iridiumQueue, &report, 1000) == pdTRUE) {
					DebugReport(&report);
					if (!isnan(report.latitude) && !isnan(report.longitude)) {
						payload = ReportToIridiumPayload(&report);

						PrintfLock();
						snprintf(buf, 24, "Iridium send %d/%d...", ++i, n);
						PrintfUnlock();

						Display_LockStatus();
						Display_SetStatus(buf);

						if (Iridium_Write(payload) == IRIDIUM_ERROR || Iridium_Transfer() == IRIDIUM_ERROR) {
							Display_SetStatus("Iridium failed!");
							osDelay(3000);
							Display_UnlockStatus();

							goto IRIDIUM_RESET;
						}

						// Take out the sent item.
						xQueueReceive(iridiumQueue, &report, 10);

						lastReportSendTime = HAL_GetTick();
						Display_UnlockStatus();
					}
				}
			}

		} else {
			Iridium->networkAvailable = 0;
			Iridium->signalQuality = 0;
		}

		IRIDIUM_RESET:

		osDelay(1000);
	}
}
