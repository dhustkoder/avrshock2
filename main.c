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

static uint8_t ps2c_tranceiver(const uint8_t data)
{
	uint8_t recv = 0x00;

	for (unsigned i = 0; i < 8; ++i) {
		if (data&_BV(i))
			PORT_STATUS |= PORT_COMMAND;
		else
			PORT_STATUS &= ~PORT_COMMAND;

		PORT_STATUS &= ~PORT_CLOCK; // clock low
		_delay_us(1);

		if (PIN_STATUS&PIN_DATA)
			recv |= _BV(i);
		
		PORT_STATUS |= PORT_CLOCK; // clock high
		_delay_us(1);
	}

	PORT_STATUS |= PORT_COMMAND;
	_delay_ms(20);

	return recv;
}


__attribute__((noreturn)) void main(void)
{
	ps2c_init();
	uart_init();

	_delay_ms(150);
	
	const uint8_t send[5] = { 0x01, 0x42, 0x00, 0x00, 0x00 };
	uint8_t recv[5];

	for (;;) {
		PORT_STATUS &= ~PORT_ATTENTION;
		_delay_us(500);

		for (unsigned i = 0; i < 5; ++i)
			recv[i] = ps2c_tranceiver(send[i]);

		PORT_STATUS |= PORT_ATTENTION;

		for (unsigned i = 0; i < 5; ++i)
			printf("- %.2X - ", recv[i]);

		printf("\n");
	}
}


