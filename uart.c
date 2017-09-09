#include <avr/io.h>

#ifndef BAUD
#define BAUD (9600)
#endif

#include <util/setbaud.h>

#include "uart.h"


FILE uartout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uartin = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);


void uart_init(void)
{
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */ 
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */

	stdout = &uartout;
	stdin = &uartin;
}


int uart_putchar(const char c, FILE* const stream)
{
	if (c == '\n')
		uart_putchar('\r', stream);

	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

int uart_getchar(FILE* const stream)
{
	((void)stream);
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}
