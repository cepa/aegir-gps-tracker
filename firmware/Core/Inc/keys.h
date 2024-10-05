/*
 * keys.h
 *
 *  Created on: Mar 18, 2024
 *      Author: cepa
 */

#ifndef INC_KEYS_H_
#define INC_KEYS_H_

#include <stdint.h>

#define KEY1_PIN	GPIO_PIN_3
#define KEY1_PORT	GPIOB

#define KEY2_PIN	GPIO_PIN_4
#define KEY2_PORT	GPIOB

#define KEY_UP	0
#define KEY_DOWN	1

typedef uint8_t Key_t;

Key_t Key1_GetState();
Key_t Key2_GetState();
void Key_TaskHandler(void const *argument);

#endif /* INC_KEYS_H_ */
