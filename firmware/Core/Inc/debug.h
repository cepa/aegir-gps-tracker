/*
 * debug.h
 *
 *  Created on: Mar 7, 2024
 *      Author: cepa
 */

#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_

#include <stdarg.h>

// https://en.wikipedia.org/wiki/Variadic_macro_in_the_C_preprocessor
#define _DEBUG(fmt, ...) __debug(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

void __debug(const char *file, int line, const char *func, const char *fmt, ...);

#endif /* INC_DEBUG_H_ */
