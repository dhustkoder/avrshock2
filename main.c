#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "uart.h"


#define DC_DOWN_LIM (20)
#define DC_UP_LIM (100)


static uint8_t dc = DC_DOWN_LIM;
static bool isr_timer0_ovf = false;


__attribute__((noreturn)) void main(void)
{
	uart_init();

	DDRD |= _BV(PORTD6);

	TCCR0A = _BV(COM0A1)|_BV(WGM00)|_BV(WGM01);
	
	TIMSK0 = _BV(TOIE0);

	OCR0A = (dc / 100.0f) * 255.0f;

	sei();

	TCCR0B = _BV(CS00);

	const uint8_t add = 1;
	const uint8_t sub = 1;

	bool up = true;
	for (;;) {
		_delay_ms(1);
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

		if (isr_timer0_ovf) {
			isr_timer0_ovf = false;
			// work around avr minprintf problem with floating format (%f)
			const float num = (dc / 100.f) * 5.f;
			const int8_t whole = (int8_t) num;
			const int8_t fract = (num - whole) * 100.f;
			printf("PWM VOLTAGE: %" PRIi8 ".%.2" PRIi8 "\r", whole, fract);
		}
	}
}


ISR(TIMER0_OVF_vect)
{	
	OCR0A = (dc / 100.0f) * 255.0f;
	isr_timer0_ovf = true;
}

