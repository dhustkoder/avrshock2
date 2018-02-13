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

static const char* const button_names[] = { 
	"SELECT", "L3", "R3", "START", "UP", "RIGHT", "DOWN", "LEFT", 
	"L2", "R2", "L1", "R1", "TRIANGLE", "CIRCLE", "CROSS", "SQUARE"
};

static const char* const axis_names[] = {
	"RX", "RY",
	"LX", "LY"
};


noreturn void main(void)
{
	avrshock2_button_t buttons = 0;
	avrshock2_axis_t axis[AVRSHOCK2_AXIS_SIZE];

	uart_init();
	printf("initializing avrshock2...\n");
	avrshock2_init();
	printf("setting mode...\n");
	avrshock2_set_mode(AVRSHOCK2_MODE_DIGITAL, false);
	printf("done!\n");
	for (;;) {
		if (avrshock2_poll(&buttons, axis)) {
			putchar(12);
			puts("\tAVRSHOCK2 EXAMPLE!\n");
			printf("Controller mode: %.2X\n", (unsigned)avrshock2_get_mode());	
			/* digital */
			for (int i = 0; i < AVRSHOCK2_BUTTON_NBUTTONS; ++i)
				printf("BUTTON %s: %d\n", button_names[i], (buttons&(1<<i)) ? 1 : 0);
			/* axis */
			for (int i = 0; i < AVRSHOCK2_AXIS_SIZE; ++i)
				printf("AXIS %s: %d\n", axis_names[i], (int)axis[i]);
		}
		_delay_ms(2);
	}
}

