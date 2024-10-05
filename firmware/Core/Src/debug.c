/*
 * debug.c
 *
 *  Created on: Mar 7, 2024
 *      Author: cepa
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "cmsis_os.h"
#include "task.h"
#include "debug.h"
#include "printf.h"

/*
 * Use simple mutex to avoid race condition in serial conole access (printf)
 * with multiple task writing in the same time.
 */
void __debug(const char *file, int line, const char *func, const char *fmt, ...)
{
	va_list args;
	TaskHandle_t task = xTaskGetCurrentTaskHandle();
	TickType_t tick = xTaskGetTickCount();
	PrintfLock();
	va_start(args, fmt);
	if (task != NULL) {
		printf("[%lu:%s]: %s:%d: %s: ", tick, pcTaskGetName(task), file, line, func);
	} else {
		printf("[%lu]: %s:%d: %s: ", tick, file, line, func);
	}
	vprintf(fmt, args);
	printf("\r\n");
	va_end(args);
	PrintfUnlock();
}
