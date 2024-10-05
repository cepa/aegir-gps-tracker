/*
 * i2c.c
 *
 *  Created on: Mar 1, 2024
 *      Author: cepa
 */

#include <stdio.h>
#include "i2c.h"
#include "debug.h"

void I2C_Scan(I2C_HandleTypeDef hi2c)
{
  HAL_StatusTypeDef res;
  uint8_t n = 0;
  _DEBUG("Scan I2C...");
  for (uint8_t i = 0; i < 128; i++) {
	res = HAL_I2C_IsDeviceReady(&hi2c, i << 1, 1, 10);
	if (res == HAL_OK) {
	  _DEBUG("Found I2C device at 0x%02x", i);
	  n++;
	}
  }
  _DEBUG("I2C scan done, found %u devices", n);
}

