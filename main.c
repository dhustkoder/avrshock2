#include <stdbool.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>
#include "uart.h"

#define PORT_MODE DDRB
#define PORT_STATUS PORTB
#define PIN_STATUS PINB

enum Ports {
	// SPI
	PORT_MOSI = _BV(PORTB3),
	PORT_MISO = _BV(PORTB4),
	PORT_SCK = _BV(PORTB5),
	PORT_SS = _BV(PORTB2),

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


static void spi_init(void)
{
	PORT_MODE &= ~PORT_MISO; // MISO input
	PORT_MODE |= PORT_MOSI|PORT_SCK|PORT_SS; // others output

	// SPI control register
	SPCR = _BV(MSTR);
}

__attribute__((noreturn)) void main(void)
{
	spi_init();

	const uint8_t ports[3] = { PORT_MOSI, PORT_SCK, PORT_SS };
	uint8_t idx = 0;
	for (;;) {
		PORT_STATUS |= ports[idx];
		_delay_ms(500);
		PORT_STATUS &= ~ports[idx];
		if (++idx == 3)
			idx = 0;

	}
}


