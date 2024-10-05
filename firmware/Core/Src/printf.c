/*
 * printf.c
 *
 *  Created on: May 8, 2024
 *      Author: cepa
 */


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "cmsis_os.h"
#include "printf.h"

static volatile uint8_t mutex = 0;

void PrintfLock()
{
	while (mutex != 0) {
		osDelay(1);
	}
	mutex = 1;
}

void PrintfUnlock()
{
	mutex = 0;
}
