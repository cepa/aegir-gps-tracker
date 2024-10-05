/*
 * keys.c
 *
 *  Created on: Mar 18, 2024
 *      Author: cepa
 */

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "keys.h"
#include "debug.h"

static Key_t key1 = KEY_UP;
static Key_t key2 = KEY_DOWN;

static Key_t GetKeyState(GPIO_TypeDef *port, uint16_t pin)
{
	// Key press pulls down.
	if (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET) {
		osDelay(50);
		if (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET) {
			return KEY_DOWN;
		}
	}
	return KEY_UP;
}

Key_t Key1_GetState()
{
	return key1;
}

Key_t Key2_GetState()
{
	return key2;
}

void Key_TaskHandler(void const *argument)
{
	Key_t state;

	for (;;) {
		state = GetKeyState(KEY1_PORT, KEY1_PIN);
		if (key1 != state) {
			if (state == KEY_DOWN) _DEBUG("Key1 down");
			if (state == KEY_UP) _DEBUG("Key1 up");
		}
		key1 = state;

		state = GetKeyState(KEY2_PORT, KEY2_PIN);
		if (key2 != state) {
			if (state == KEY_DOWN) _DEBUG("Key2 down");
			if (state == KEY_UP) _DEBUG("Key2 up");
		}
		key2 = state;

		osDelay(50);
	}
}
