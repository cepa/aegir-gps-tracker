/*
 * display.c
 *
 *  Created on: Mar 21, 2024
 *      Author: cepa
 */

#include <stdio.h>
#include "cmsis_os.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "debug.h"
#include "display.h"
#include "gps.h"
#include "lte.h"
#include "iridium.h"
#include "printf.h"

static volatile const char *__status = NULL;
static volatile int __statusLock = 0;

void Display_Init()
{
	ssd1306_Init();
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
}

void Display_SetStatus(const char *status)
{
	__status = status;
}

const char *Display_GetStatus()
{
	return __status;
}

void Display_LockStatus()
{
	__statusLock = 1;
}

void Display_UnlockStatus()
{
	__statusLock = 0;
}

int Display_StatusIsLocked()
{
	return __statusLock;
}

void Display_RenderBootScreen()
{
	ssd1306_Fill(Black);
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("Aegir GPS Tracker", Font_6x8, White);
	ssd1306_SetCursor(0, 8);
	ssd1306_WriteString("Booting...", Font_6x8, White);
	ssd1306_UpdateScreen();
}

void Display_RenderDefaultScreen()
{
	char buf[24];
	float lat, lon;

	ssd1306_Fill(Black);

	ssd1306_Line(0, 9, 127, 9, White);
	ssd1306_Line(64, 9, 64, 53, White);
	ssd1306_Line(0, 53, 127, 53, White);

	PrintfLock();

	snprintf(buf, 24, "20%02d-%02d-%02d   %02d:%02d:%02d",
			GPS->date.year,
			GPS->date.month,
			GPS->date.day,
			GPS->time.hours,
			GPS->time.minutes,
			GPS->time.seconds);
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString(buf, Font_6x8, White);

	lat = minmea_tocoord(&GPS->latitude);
	lon = minmea_tocoord(&GPS->longitude);

	snprintf(buf, 24, "%0.5f%c", lat >= 0.0 ? lat : -lat, lat >= 0.0 ? 'N' : 'S');
	ssd1306_SetCursor(0, 12);
	ssd1306_WriteString(buf, Font_6x8, White);

	snprintf(buf, 24, "%0.5f%c",lon >= 0.0 ? lon : -lon, lon >= 0.0 ? 'E' : 'W');
	ssd1306_SetCursor(0, 20);
	ssd1306_WriteString(buf, Font_6x8, White);

	snprintf(buf, 24, "%0.2fkn", minmea_tofloat(&GPS->speed));
	ssd1306_SetCursor(0, 28);
	ssd1306_WriteString(buf, Font_6x8, White);

	snprintf(buf, 24, "%ddeg", (int) minmea_tofloat(&GPS->course));
	ssd1306_SetCursor(0, 36);
	ssd1306_WriteString(buf, Font_6x8, White);

	snprintf(buf, 24, "%dm", (int) minmea_tofloat(&GPS->altitude));
	ssd1306_SetCursor(0, 44);
	ssd1306_WriteString(buf, Font_6x8, White);

	snprintf(buf, 24, "G%d L%d I%d",
			GPS->satellites,
			LTE->signalQuality,
			Iridium->networkAvailable + Iridium->signalQuality);
	ssd1306_SetCursor(66, 12);
	ssd1306_WriteString(buf, Font_6x8, White);

	uint32_t tick = HAL_GetTick();
	snprintf(buf, 24, "%dh%02dm%02ds", tick / 1000 / 3600, (tick / 1000 / 60) % 60, (tick / 1000) % 60);
	ssd1306_SetCursor(66, 20);
	ssd1306_WriteString(buf, Font_6x8, White);

	if (__status != NULL) {
		ssd1306_SetCursor(0, 56);
		ssd1306_WriteString(__status, Font_6x8, White);
	}

	PrintfUnlock();

	ssd1306_UpdateScreen();
}

void Display_TaskHandler(void const *argument)
{
	osDelay(1000);

	for (;;) {
		Display_RenderDefaultScreen();
		osDelay(300);
	}
}
