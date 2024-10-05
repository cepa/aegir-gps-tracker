/*
 * sensor.c
 *
 *  Created on: Mar 20, 2024
 *      Author: cepa
 */

#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "debug.h"
#include "sensor.h"

void Sensor_TaskHandler(void const *argument)
{
	for (;;) {
		//_DEBUG("sensor");
		osDelay(1000);
	}
}
