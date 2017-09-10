#include <stdbool.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
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
	PORT_STATUS |= PORT_MISO; // enable pull up
	PORT_STATUS |= PORT_SS; // must be high
	// set SPI as master
	// prescaler Fosc / 16
	// enable interrupts
	// enable SPI
	SPCR = _BV(MSTR)|_BV(SPR0)|_BV(SPR1)|_BV(SPE);
}

static uint8_t spi_tranceiver(const uint8_t data)
{
	SPDR = data;

	while (!(SPSR&_BV(SPIF))) ;

	return SPDR;
}

__attribute__((noreturn)) void main(void)
{
	spi_init();
	uart_init();
	
	const uint8_t send[5] = { 0x01, 0x42, 0x00, 0x00, 0x00 };
	uint8_t recv[5];

	for (;;) {
		PORT_STATUS &= ~PORT_ATTENTION;

		for (unsigned i = 0; i < 5; ++i)
			recv[i] = spi_tranceiver(send[i]);

		PORT_STATUS |= PORT_ATTENTION;

		for (unsigned i = 0; i < 5; ++i)
			printf("- %.2X - ", recv[i]);

		printf("\n");
	}
}


