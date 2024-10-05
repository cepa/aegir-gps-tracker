/*
 * lte.c
 *
 *  Created on: Mar 18, 2024
 *      Author: cepa
 */

#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "lte.h"
#include "debug.h"
#include "report.h"
#include "display.h"

//#define COBAN_IP	"178.214.4.5"
//#define COBAN_PORT	9999
#define COBAN_IP	"217.182.76.191"
#define COBAN_PORT	9000

static LTE_t __context;
LTE_t *LTE = &__context;
extern UART_HandleTypeDef LTE_UART;
QueueHandle_t lteQueue;

LTE_Status_t LTE_Init()
{
	LTE_Status_t status = LTE_ERROR;
	int i;
	const char *reply;

	memset(LTE, 0, sizeof(LTE_t));
	ATModem_Init(&LTE->atm, &LTE_UART);

	// @TODO: Hardware reset/init here?

	// Wait for the module to boot
	osDelay(5000);

	// ESC just in case
	ATModem_SendCommand(&LTE->atm, "\x1b\r\n");

	// Init modem, n times to unlock modem if stuck.
	for (i = 0; i < 30; i++) {
		ATModem_SendCommand(&LTE->atm, "AT\r\n");
		reply = ATModem_WaitForReply(&LTE->atm, 1000);
		if (reply != NULL && strstr(reply, "OK") != NULL) {
			status = LTE_OK;
			break;
		}
	}
	if (status == LTE_ERROR) {
		return status;
	}

	// Get modem info
	ATModem_SendCommand(&LTE->atm, "ATI\r\n");
	ATModem_WaitForReply(&LTE->atm, 3000);

	// Echo mode 0
	ATModem_SendCommand(&LTE->atm, "ATE0\r\n");
	ATModem_WaitForReply(&LTE->atm, 3000);

	// Echo mode 0
	ATModem_SendCommand(&LTE->atm, "AT+CMEE=0\r\n");
	ATModem_WaitForReply(&LTE->atm, 3000);

	// Info
	ATModem_SendCommand(&LTE->atm, "AT+CPSI?\r\n");
	ATModem_WaitForReply(&LTE->atm, 3000);

	// Choose LTE network selection
	ATModem_SendCommand(&LTE->atm, "AT+CNMP=38\r\n");
	ATModem_WaitForReply(&LTE->atm, 3000);

	// Don't wait for TCP ACK
	ATModem_SendCommand(&LTE->atm, "AT+CIPSENDMODE=0\r\n");
	ATModem_WaitForReply(&LTE->atm, 3000);

	// Timeout 120s
	ATModem_SendCommand(&LTE->atm, "AT+CIPTIMEOUT=120000,120000,120000\r\n");
	ATModem_WaitForReply(&LTE->atm, 3000);

	return status;
}

LTE_Status_t LTE_Reset()
{
	ATModem_SendCommand(&LTE->atm, "AT+CRESET\r\n");
	ATModem_WaitForReply(&LTE->atm, 3000);

	return LTE_OK;
}

uint8_t LTE_SIMIsReady()
{
	const char *reply = NULL;
	ATModem_SendCommand(&LTE->atm, "AT+CPIN?\r\n");
	reply = ATModem_WaitForReply(&LTE->atm, 10000);
	return (reply != NULL && strstr(reply, "CPIN: READY") != NULL) ? 1 : 0;
}

uint8_t LTE_NetworkIsReady()
{
	const char *tokens[] = {"+CREG: 0,5", "+CREG: 1,5", "+CREG: 2,5"};
	const char *reply = NULL;
	ATModem_SendCommand(&LTE->atm, "AT+CREG?\r\n");
	reply = ATModem_WaitForReply(&LTE->atm, 3000);
	if (reply != NULL) {
		for (int i = 0; i < 3; i++) {
			if (strstr(reply, tokens[i]) != NULL) {
				return 1;
			}
		}
	}
	return 0;
}

uint8_t LTE_NetIsOpened()
{
	const char *reply = NULL;
	ATModem_SendCommand(&LTE->atm, "AT+NETOPEN?\r\n");
	reply = ATModem_WaitForReply(&LTE->atm, 10000);
	return (reply != NULL && strstr(reply, "+NETOPEN: 1") != NULL) ? 1 : 0;
}

LTE_Status_t LTE_WaitForSIMReady()
{
	int retries = 10;
	do {
		if (LTE_SIMIsReady() == 1) {
			return LTE_OK;
		}
		osDelay(1000);
	} while (retries-- > 0);
	return LTE_ERROR;
}

LTE_Status_t LTE_WaitForNetworkReady()
{
	int attempts = 3;
	int retries = 120;
	do {
		// Request network registration (2; with lac and ci)
		ATModem_SendCommand(&LTE->atm, "AT+CREG=2\r\n");
		ATModem_WaitForReply(&LTE->atm, 30000);
		do {
			if (LTE_NetworkIsReady() == 1) {
				LTE->networkStatus = LTE_NETWORK_REGISTERED;
				return LTE_OK;
			}
			LTE->networkStatus = LTE_NETWORK_UNREGISTERED;
			osDelay(1000);
		} while (retries-- > 0);
	} while (attempts-- > 0);
	return LTE_ERROR;
}

LTE_Status_t LTE_UpdateSignalQuality()
{
	const char *reply = NULL;
	ATModem_SendCommand(&LTE->atm, "AT+CSQ\r\n");
	reply = ATModem_WaitForReply(&LTE->atm, 30000);
	if (reply != NULL && (reply = strstr(reply, "+CSQ:")) != NULL) {
		sscanf(reply, "+CSQ: %d", &LTE->signalQuality);
		_DEBUG("LTE signal quality: %d", LTE->signalQuality);
		return LTE_OK;
	}
	LTE->signalQuality = 0;
	return LTE_ERROR;
}

LTE_Status_t LTE_NetOpen(const char *apn, const char *username, const char *password)
{
	const char *reply = NULL;
	int retries = 120;

	// Clean up
	//LTE_NetDisconnect();

	// Configure PDP context
	ATModem_SendCommand(&LTE->atm, "AT+CGDCONT=1,\"IP\",\"%s\",\"%s\",\"%s\"\r\n", apn, username, password);
	ATModem_WaitForReply(&LTE->atm, 30000);

	// Enable non transparent mode
	ATModem_SendCommand(&LTE->atm, "AT+CIPMODE=0\r\n");
	ATModem_WaitForReply(&LTE->atm, 30000);

	// Activate context
	ATModem_SendCommand(&LTE->atm, "AT+NETOPEN\r\n");
	ATModem_WaitForReply(&LTE->atm, 120000);

	do {
		ATModem_SendCommand(&LTE->atm, "AT+NETOPEN?\r\n");
		reply = ATModem_WaitForReply(&LTE->atm, 3000);
		if (reply != NULL) {
			if (strstr(reply, "+NETOPEN: 1") != NULL) {
				return LTE_OK;
			}
		}

		osDelay(1000);

	} while (retries-- > 0);

	return LTE_ERROR;
}

LTE_Status_t LTE_NetClose()
{
	// Deactivate context
	ATModem_SendCommand(&LTE->atm, "AT+NETCLOSE");
	ATModem_WaitForReply(&LTE->atm, 30000);

	return LTE_OK;
}

uint8_t LTE_IsConnected(const char *addr, int port)
{
	const char *reply = NULL;
	char tmp[64];

	// Expected reply
	PrintfLock();
	snprintf(tmp, 64, "+CIPOPEN: 0,\"TCP\",\"%s\",%d,-1", addr, port);
	PrintfUnlock();

	// Check if already connected
	ATModem_SendCommand(&LTE->atm, "AT+CIPOPEN?\r\n");
	reply = ATModem_WaitForReply(&LTE->atm, 3000);
	if (reply != NULL) {
		if (strstr(reply, tmp) != NULL) {
			return 1;
		}
	}

	return 0;
}

LTE_Status_t LTE_Connect(const char *addr, int port)
{
	const char *reply = NULL;
	const char *expected[] = {"CONNECT OK", "CONNECT FAIL", "ALREADY CONNECT", "ERROR", "CLOSE OK", "OK"};
	int retries = 30;
	char tmp[64];

	// Expected reply
	PrintfLock();
	snprintf(tmp, 64, "+CIPOPEN: 0,\"TCP\",\"%s\",%d,-1", addr, port);
	PrintfUnlock();

	// Check if already connected
	ATModem_SendCommand(&LTE->atm, "AT+CIPOPEN?\r\n");
	reply = ATModem_WaitForReply(&LTE->atm, 3000);
	if (reply != NULL) {
		if (strstr(reply, tmp) != NULL) {
			return LTE_OK;
		}
	}


	// Open connection
	ATModem_SendCommand(&LTE->atm, "AT+CIPOPEN=0,\"TCP\",\"%s\",%d\r\n", addr, port);
	reply = ATModem_WaitForExpectedReply(&LTE->atm, expected, 6, 120000);
	if (reply != NULL) {
		if (strstr(reply, "ERROR") != NULL) {
			return LTE_ERROR;
		}
	}

	// Wait for connection
	do {
		ATModem_SendCommand(&LTE->atm, "AT+CIPOPEN?\r\n");
		reply = ATModem_WaitForReply(&LTE->atm, 3000);
		if (reply != NULL) {
			if (strstr(reply, tmp) != NULL) {
				return LTE_OK;
			}
			if (strstr(reply, "ERROR") != NULL) {
				return LTE_ERROR;
			}
		}

		osDelay(1000);
	} while (retries-- > 0);

	return LTE_ERROR;
}

LTE_Status_t LTE_Close()
{
	// Close connection
	ATModem_SendCommand(&LTE->atm, "AT+CIPCLOSE=0\r\n");
	ATModem_WaitForReply(&LTE->atm, 30000);
	return LTE_OK;
}

LTE_Status_t LTE_Send(const char *payload)
{
	int retries = 3; // n * 10s
	const char *reply = NULL;

	// Send begin
	ATModem_SendCommand(&LTE->atm, "AT+CIPSEND=0,%d\r\n", strlen(payload));
	reply = ATModem_WaitForExpectedReply(&LTE->atm, (const char *[]){">", "ERROR"}, 2, 120000);
	if (reply != NULL) {
		if (strstr(reply, "ERROR") != NULL) {
			return LTE_ERROR;
		}
	} else {
		// ESC just in case
		ATModem_SendCommand(&LTE->atm, "\x1b\r\n");
		return LTE_ERROR;
	}

	do {
		// Send payload
		osDelay(10);
		ATModem_SendPayload(&LTE->atm, payload);

		// Wait for finish, 10s and then retry sending the payload
		reply = ATModem_WaitForExpectedReply(&LTE->atm, (const char *[]){"+CIPSEND: 0,", "+CIPERROR:"}, 2, 10000);
		if (reply != NULL) {
			if (strstr(reply, "+CIPSEND") != NULL) {
				return LTE_OK;
			}
			if (strstr(reply, "+CIPERROR") != NULL) {
				// ESC just in case
				ATModem_SendCommand(&LTE->atm, "\x1b\r\n");
				return LTE_ERROR;
			}
		}

	} while (retries-- > 0);

	return LTE_ERROR;
}

void LTE_UART_Callback()
{
	ATModem_UART_Callback(&LTE->atm);
}

extern int iridiumCounter;
void LTE_TaskHandler(void const *argument)
{
	Report_t report;
	char buf[24];
	const char *payload = NULL;
	int i = 0, n = 0;

	for (;;) {

		if (LTE_Init() == LTE_ERROR) {
			osDelay(120000);
			continue;
		}

		if (LTE_WaitForSIMReady() == LTE_ERROR) {
			osDelay(120000);
			continue;
		}

		if (LTE_WaitForNetworkReady() == LTE_ERROR) {
			osDelay(120000);
			continue;
		}

		LTE_UpdateSignalQuality();

		if (!LTE_NetIsOpened()) {
			LTE_NetOpen("iot.1nce.net", "", "");
		}

		if (LTE_NetIsOpened()) {
			if (LTE_Connect(COBAN_IP, COBAN_PORT) == LTE_OK) {
				while (LTE_IsConnected(COBAN_IP, COBAN_PORT)) {
					LTE_UpdateSignalQuality();
					if (LTE->signalQuality == 0) {
						goto LTE_RESET;
					}

					i = 0;
					while ((n = uxQueueMessagesWaiting(lteQueue)) > 0) {
						if (xQueueReceive(lteQueue, &report, 1000) == pdTRUE) {

							DebugReport(&report);
							payload = ReportToCobanPayload(&report);
							if (payload != NULL) {

								PrintfLock();
								snprintf(buf, 24, "LTE send %d/%d...", ++i, n);
								PrintfUnlock();

								Display_LockStatus();
								Display_SetStatus(buf);

								if (LTE_Send(payload) == LTE_ERROR) {
									Display_SetStatus("LTE failed!");
									osDelay(3000);
									Display_UnlockStatus();

									goto LTE_RESET;
								}

								// Reset iridium counter
								iridiumCounter = 0;

								lastReportSendTime = HAL_GetTick();
								Display_UnlockStatus();
							}
						}
					}

					osDelay(10000);
				}
			}
		}

		LTE_RESET:
		// Cleanup
		//LTE_Close();
		//LTE_NetClose();
		LTE_Reset();

		// Wait 39s for LTE reset
		osDelay(30000);
	}
}
