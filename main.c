#include <stdbool.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "uart.h"


#define DC_DOWN_LIM (20)
#define DC_UP_LIM (100)


static uint8_t dc = DC_DOWN_LIM;


__attribute__((noreturn)) void main(void)
{
	uart_init();

	DDRD |= _BV(PORTD6);

	TCCR0A = _BV(COM0A1)|_BV(WGM00)|_BV(WGM01);
	
	TIMSK0 = _BV(TOIE0);

	OCR0A = (dc / 100.0f) * 255.0f;

	sei();

	TCCR0B = _BV(CS00);

	const uint8_t add = 24;
	const uint8_t sub = 24;

	bool up = true;
	for (;;) {
		_delay_us(0.0001);
		if (up) {
			if ((dc + add) > DC_UP_LIM)
				up = false;
			else
				dc += add;
		} else {
			if ((dc - sub) < DC_DOWN_LIM)
				up = true;
			else
				dc -= sub;
		}
	}
}


ISR(TIMER0_OVF_vect)
{	
	OCR0A = (dc / 100.0f) * 255.0f;
}

