#ifndef AVRSHOCK2_UART_H_
#define AVRSHOCK2_UART_H_
#include <stdio.h>


void uart_init(void);
int uart_putchar(const char c, FILE* stream);
int uart_getchar(FILE* stream);


#endif
