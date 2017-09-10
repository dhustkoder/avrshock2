#include <stdbool.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>
#include "uart.h"

enum Ports {
	PORT_DATA = ,
	PORT_COMMAND =  ,
	PORT_ATTENTION = ,
	PORT_CLOCK = 
};

enum Pins {
	PIN_DATA = ,
	PIN_COMMAND = ,
	PIN_ATTENTION = ,
	PIN_CLOCK =
};


__attribute__((noreturn)) void main(void)
{

	for (;;) {
		_delay_ms(500);
	}
}


