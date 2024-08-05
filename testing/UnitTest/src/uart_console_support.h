/*
 * uart_support.h
 *
 * Created: 7/29/2024 10:45:26 AM
 *  Author: lydia.klaus
 */ 


#ifndef UART_SUPPORT_H_
#define UART_SUPPORT_H_

#include <stddef.h>
#include "sam.h"

/**
 *  Configure UART console.
 */
void configure_console(void);
void uart_writechar(char a);
void uart_writestr(const char* str);


#endif /* UART_SUPPORT_H_ */