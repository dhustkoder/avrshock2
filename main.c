#include <util/delay.h>
#include "uart.h"


__attribute__((noreturn)) void main(void)
{
	uart_init();

	for (;;) {
		_delay_ms(500);
		puts("Hello World!");
		_delay_ms(500);
	}
}


