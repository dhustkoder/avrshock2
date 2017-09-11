#ifndef PS2C_UART_H_
#define PS2C_UART_H_
#include <stdio.h>


void uart_init(void);
int uart_putchar(const char c, FILE* stream);
int uart_getchar(FILE* stream);


#endif
