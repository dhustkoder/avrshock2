#include <string.h>
#include <stdnoreturn.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "uart.h"
#include "avrshock2.h"

/* *
 * By Rafael Moura 2017 (https://github.com/dhustkoder)
 * avrshock2 usage example
 * uncomment uart_init and printfs for serial information on your computer
 * set the BAUD rate in the makefile.
 * this example was tested on atmega328p Arduino UNO board
 * */

static const char* const button_name[] = {
	[AVRSHOCK2_BUTTON_SELECT] = "SELECT",
	[AVRSHOCK2_BUTTON_L3]     = "L3",
	[AVRSHOCK2_BUTTON_R3]     = "R3",
	[AVRSHOCK2_BUTTON_START]  = "START",
	[AVRSHOCK2_BUTTON_UP]     = "UP",
	[AVRSHOCK2_BUTTON_RIGHT]  = "RIGHT",
	[AVRSHOCK2_BUTTON_DOWN]   = "DOWN",
	[AVRSHOCK2_BUTTON_LEFT]   = "LEFT",
	[AVRSHOCK2_BUTTON_L2]     = "L2",
	[AVRSHOCK2_BUTTON_R2]     = "R2",
	[AVRSHOCK2_BUTTON_L1]     = "L1",
	[AVRSHOCK2_BUTTON_R1]     = "R1",
	[AVRSHOCK2_BUTTON_TRI]    = "TRIANGLE",
	[AVRSHOCK2_BUTTON_CIRCLE] = "CIRCLE",
	[AVRSHOCK2_BUTTON_CROSS]  = "CROSS",
	[AVRSHOCK2_BUTTON_SQUARE] = "SQUARE"
};

static const char* const analog_name[] = {
	[AVRSHOCK2_ANALOG_RX] = "RX",
	[AVRSHOCK2_ANALOG_RY] = "RY",
	[AVRSHOCK2_ANALOG_LX] = "LX",
	[AVRSHOCK2_ANALOG_LY] = "LY"
};


noreturn void main(void)
{
	avrshock2_init();
	uart_init();

	/* let's play with LEDs using the analog stick */
	/*
	#define SENSITIVITY (80)
	DDRD |= (1<<PD3)|(1<<PD5)|(1<<PD6);
	DDRB |= (1<<PB1);
	*/

	avrshock2_set_mode(AVRSHOCK2_MODE_ANALOG, true);

	for (;;) {
		avrshock2_poll();
		/*
		if (avrshock2_buttons[AVRSHOCK2_BUTTON_L3]) {
			PORTD |= (1<<PD3)|(1<<PD5)|(1<<PD6);
			PORTB |= (1<<PB1);
			continue;
		}

		if (avrshock2_analogs[AVRSHOCK2_ANALOG_LX] > (128 + SENSITIVITY))
			PORTD |= (1<<PD3);
		else
			PORTD &= ~(1<<PD3);

		if (avrshock2_analogs[AVRSHOCK2_ANALOG_LX] < (128 - SENSITIVITY))
			PORTD |= (1<<PD5);
		else
			PORTD &= ~(1<<PD5);

		if (avrshock2_analogs[AVRSHOCK2_ANALOG_LY] > (128 + SENSITIVITY))
			PORTD |= (1<<PD6);
		else
			PORTD &= ~(1<<PD6);

		if (avrshock2_analogs[AVRSHOCK2_ANALOG_LY] < (128 - SENSITIVITY))
			PORTB |= (1<<PB1);
		else
			PORTB &= ~(1<<PB1);

		*/	
		putchar(12);
		puts("\tAVRSHOCK2 EXAMPLE!\n");
		printf("MODE: $%.2X\n", avrshock2_currmode());
		for (avrshock2_analog_t i = AVRSHOCK2_ANALOG_FIRST; i <= AVRSHOCK2_ANALOG_LAST; ++i)
			printf("%s: %d\n", analog_name[i], avrshock2_analogs[i]);
		for (avrshock2_button_t i = AVRSHOCK2_BUTTON_FIRST; i <= AVRSHOCK2_BUTTON_LAST; ++i)
			printf("%s: %d\n", button_name[i], avrshock2_buttons[i]);
		_delay_ms(850);
	}
}

