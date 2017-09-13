#include <string.h>
#include <stdnoreturn.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "ps2c.h"
#include "uart.h"

/* *
 * By Rafael Moura 2017 (https://github.com/dhustkoder)
 * ps2c usage example
 * uncomment uart_init and printfs for serial information on your computer
 * set the BAUD rate in the makefile.
 * this example was tested on atmega328p Arduino UNO board
 * */

static const char* const button_name[] = {
	[PS2C_BUTTON_SELECT] = "SELECT",
	[PS2C_BUTTON_L3]     = "L3",
	[PS2C_BUTTON_R3]     = "R3",
	[PS2C_BUTTON_START]  = "START",
	[PS2C_BUTTON_UP]     = "UP",
	[PS2C_BUTTON_RIGHT]  = "RIGHT",
	[PS2C_BUTTON_DOWN]   = "DOWN",
	[PS2C_BUTTON_LEFT]   = "LEFT",
	[PS2C_BUTTON_L2]     = "L2",
	[PS2C_BUTTON_R2]     = "R2",
	[PS2C_BUTTON_L1]     = "L1",
	[PS2C_BUTTON_R1]     = "R1",
	[PS2C_BUTTON_TRI]    = "TRIANGLE",
	[PS2C_BUTTON_CIR]    = "CIRCLE",
	[PS2C_BUTTON_X]      = "X",
	[PS2C_BUTTON_SQR]    = "SQUARE"
};

static const char* const analog_name[] = {
	[PS2C_ANALOG_RX] = "RX",
	[PS2C_ANALOG_RY] = "RY",
	[PS2C_ANALOG_LX] = "LX",
	[PS2C_ANALOG_LY] = "LY"
};


noreturn void main(void)
{
	ps2c_init();
	/* uart_init(); */

	/* let's play with LEDs using the analog stick */
	#define SENSITIVITY (80)
	DDRD |= (1<<PD3)|(1<<PD5)|(1<<PD6);
	DDRB |= (1<<PB1);

	ps2c_set_mode(PS2C_MODE_ANALOG, true);

	for (;;) {
		ps2c_poll();

		if (ps2c_buttons[PS2C_BUTTON_L3]) {
			PORTD |= (1<<PD3)|(1<<PD5)|(1<<PD6);
			PORTB |= (1<<PB1);
			continue;
		}

		if (ps2c_analogs[PS2C_ANALOG_LX] > (128 + SENSITIVITY))
			PORTD |= (1<<PD3);
		else
			PORTD &= ~(1<<PD3);

		if (ps2c_analogs[PS2C_ANALOG_LX] < (128 - SENSITIVITY))
			PORTD |= (1<<PD5);
		else
			PORTD &= ~(1<<PD5);

		if (ps2c_analogs[PS2C_ANALOG_LY] > (128 + SENSITIVITY))
			PORTD |= (1<<PD6);
		else
			PORTD &= ~(1<<PD6);

		if (ps2c_analogs[PS2C_ANALOG_LY] < (128 - SENSITIVITY))
			PORTB |= (1<<PB1);
		else
			PORTB &= ~(1<<PB1);

		/*
		putchar(12);
		printf("MODE: $%.2X\n", ps2c_currmode());
		for (uint8_t i = PS2C_ANALOG_FIRST; i <= PS2C_ANALOG_LAST; ++i)
			printf("%s: %d\n", analog_name[i], ps2c_analogs[i]);
		for (uint8_t i = PS2C_BUTTON_FIRST; i <= PS2C_BUTTON_LAST; ++i)
			printf("%s: %d\n", button_name[i], ps2c_buttons[i]);
		_delay_ms(850);
		*/
	}
}

