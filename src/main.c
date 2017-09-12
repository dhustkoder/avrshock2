#include <string.h>
#include <stdnoreturn.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "uart.h"
#include "ps2c.h"


noreturn void main(void)
{
	ps2c_init();
	uart_init();

	/* let's play with LEDs using the analog stick */
	#define SENSITIVITY (80)
	DDRD |= (1<<PD3)|(1<<PD5)|(1<<PD6);
	DDRB |= (1<<PB1);

	ps2c_set_mode(PS2C_MODE_ANALOG_PRESSURE, true);

	for (;;) {
		ps2c_analog_poll();
		
		putchar(12);
		printf("\n\nMODE: $%.2X\n", ps2c_currmode());
		printf("ANALOG JOYS [%.3d %.3d %.3d %.3d]\n",
		       ps2c_analogs[PS2C_ANALOG_RX], ps2c_analogs[PS2C_ANALOG_RY],
		       ps2c_analogs[PS2C_ANALOG_LX], ps2c_analogs[PS2C_ANALOG_LY]);
		
		for (uint8_t i = PS2C_BUTTON_FIRST; i <= PS2C_BUTTON_LAST; ++i)
			printf(" %.3d ", ps2c_buttons[i]);

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
	}
}

