#include <stdbool.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "uart.h"


/* TODO:
 * for now only bit bang implementation is done, need to add 
 * implementation for AVR SPI hardware mode instead of software bit bang
 * */

/* NOTE:
 * PORT_DATA needs a pull-up resistor around 1K - 10K
 * */


#define PORT_MODE DDRB
#define PORT_STATUS PORTB
#define PIN_STATUS PINB


enum Ports {
	// SPI
	PORT_MOSI = _BV(PB3),
	PORT_MISO = _BV(PB4),
	PORT_SCK = _BV(PB5),
	PORT_SS = _BV(PB2),

	// PS2 controller
	PORT_COMMAND =  PORT_MOSI,
	PORT_DATA = PORT_MISO,
	PORT_CLOCK = PORT_SCK,
	PORT_ATTENTION = PORT_SS
};

enum Pins {
	// SPI
	PIN_MOSI = _BV(PINB3),
	PIN_MISO = _BV(PINB4),
	PIN_SCK = _BV(PINB5),
	PIN_SS = _BV(PINB2),

	// PS2 controller
	PIN_COMMAND = PIN_MOSI,
	PIN_DATA = PIN_MISO,
	PIN_CLOCK = PIN_SCK,
	PIN_ATTENTION = PIN_SS
};


static void ps2c_init(void)
{
	PORT_MODE &= ~PORT_DATA; // DATA input
	PORT_MODE |= PORT_COMMAND|PORT_ATTENTION|PORT_CLOCK; // others out
	PORT_STATUS |= PORT_ATTENTION;
}

static void ps2c_exchange(const uint8_t size,
                          const uint8_t* const restrict send,
			  uint8_t* const restrict recv)
{
	const double rwdelay = ((1.0 / F_PS2C) * 1000000.0) / 2.0f;
	const double waitdelay = rwdelay * 10.0;

	PORT_STATUS &= ~PORT_ATTENTION;
	_delay_us(waitdelay);

	for (uint8_t i = 0; i < size; ++i) {
		const uint8_t sendbyte = send[i];
		uint8_t recvbyte = 0x00;

		for (unsigned b = 0; b < 8; ++b) {
			if (sendbyte&(0x01<<b))
				PORT_STATUS |= PORT_COMMAND;
			else
				PORT_STATUS &= ~PORT_COMMAND;

			PORT_STATUS &= ~PORT_CLOCK;
			_delay_us(rwdelay);

			if (PIN_STATUS&PIN_DATA)
				recvbyte |= (0x01<<b);

			PORT_STATUS |= PORT_CLOCK;
			_delay_us(rwdelay);
		}

		recv[i] = recvbyte;

		PORT_STATUS |= PORT_COMMAND;
		_delay_us(waitdelay);
	}

	PORT_STATUS |= PORT_ATTENTION;
	_delay_us(waitdelay);
}


__attribute__((noreturn)) void main(void)
{
	ps2c_init();
	uart_init();
	
	// play with LED's on PORTD with directional buttons
	DDRD |= _BV(PORTD2)|_BV(PORTD3)|_BV(PORTD4)|_BV(PORTD5);

	const uint8_t send[5] = { 0x01, 0x42, 0x00, 0x00, 0x00 };
	uint8_t recv[5];

	for (;;) {
		ps2c_exchange(5, send, recv);

		for (unsigned i = 0; i < 5; ++i)
			printf("- %.2X - ", recv[i]);
		printf("\n");

		const uint8_t direct = recv[3];

		if (!(direct&0x20))
			PORTD |= _BV(PORTD2);
		else
			PORTD &= ~_BV(PORTD2);

		if (!(direct&0x80))
			PORTD |= _BV(PORTD3);
		else
			PORTD &= ~_BV(PORTD3);

		if (!(direct&0x10))
			PORTD |= _BV(PORTD4);
		else
			PORTD &= ~_BV(PORTD4);

		if (!(direct&0x40))
			PORTD |= _BV(PORTD5);
		else
			PORTD &= ~_BV(PORTD5);
	}
}
