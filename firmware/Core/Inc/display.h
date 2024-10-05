/*
 * display.h
 *
 *  Created on: Mar 21, 2024
 *      Author: cepa
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

void Display_Init();
void Display_SetStatus(const char *status);
const char *Display_GetStatus();
void Display_LockStatus();
void Display_UnlockStatus();
int Display_StatusIsLocked();
void Display_RenderBootScreen();
void Display_RenderDefaultScreen();
void Display_TaskHandler(void const *argument);

#endif /* INC_DISPLAY_H_ */
