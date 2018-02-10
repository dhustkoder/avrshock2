#include <stdbool.h>
#include <string.h>
#include <stdnoreturn.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "avrshock2.h"

/* *
 *
 * By Rafael Moura 2017 (https://github.com/dhustkoder)
 *
 * TODO: (hardware spi)
 * for now only bit bang implementation is done, need to add 
 * implementation for AVR SPI hardware mode instead of software bit bang
 *
 * TODO: (pressure and motor support)
 * Couldn't properly control pressure mode, need more information about 
 * the controller commands, and need add vibration motor controll
 *
 * NOTE:
 * PIN_DATA needs a pull-up resistor around 1K - 10K
 *
 * */

#define PORT_ATT   PORTD
#define DDR_ATT    DDRD
#define BIT_ATT    (0x01<<PD2)

#define PORT_CMD   PORTD
#define DDR_CMD    DDRD
#define BIT_CMD    (0x01<<PD4)

#define PORT_DATA  PORTD
#define DDR_DATA   DDRD
#define PIN_DATA   PIND
#define BIT_DATA   (0x01<<PD7)

#define PORT_CLK   PORTB
#define DDR_CLK    DDRB
#define BIT_CLK    (0x01<<PB0)

#ifndef F_AVRSHOCK2
#define F_AVRSHOCK2 250000UL
#endif

#define RW_DELAY   (((1.0 / F_AVRSHOCK2) * 1000000.0) / 2.0)
#define WAIT_DELAY (10.0)

/* enum Button byte/bit index on avrshock2_data_buffer */
#define BUTTON_BYTE(button) avrshock2_data_buffer[(3 + (button > 7))]
#define BUTTON_BIT(button)  BUTTON_BYTE(button)&(1<<(button&0x07))


uint8_t avrshock2_buttons[AVRSHOCK2_BUTTON_LAST + 1];
uint8_t avrshock2_analogs[AVRSHOCK2_ANALOG_LAST + 1];
uint8_t avrshock2_data_buffer[34];


static uint8_t avrshock2_exchange(const uint8_t out)
{
	uint8_t in = 0x00;
	for (uint8_t b = 0; b < 8; ++b) {
		if (out&(0x01<<b))
			PORT_CMD |= BIT_CMD;
		else
			PORT_CMD &= ~BIT_CMD;

		PORT_CLK &= ~BIT_CLK;
		_delay_us(RW_DELAY);

		if (PIN_DATA&BIT_DATA)
			in |= (0x01<<b);

		PORT_CLK |= BIT_CLK;
		_delay_us(RW_DELAY);
	}

	PORT_CMD |= BIT_CMD;
	_delay_us(WAIT_DELAY);
	return in;
}

static void avrshock2_cmd(const uint8_t* const restrict cmd, const uint8_t cmdsize)
{
	PORT_ATT &= ~BIT_ATT;
	_delay_us(WAIT_DELAY);

	/* send first 2 */
	avrshock2_data_buffer[0] = avrshock2_exchange(0x01);   /* the first byte in header is always 0x01 */
	avrshock2_data_buffer[1] = avrshock2_exchange(cmd[0]); /* send second byte and get current mode   */
	/* calculate the total number of bytes to send and receive based on the mode */
	const uint8_t recvsize = ((avrshock2_data_buffer[1]&0x0F) * 2) + 3;

	/* send and receive the rest of the data */
	uint8_t i = 2;
	for (; i <= cmdsize; ++i)
		avrshock2_data_buffer[i] = avrshock2_exchange(cmd[i - 1]);
	for (; i < recvsize; ++i)
		avrshock2_data_buffer[i] = avrshock2_exchange(0x00);

	PORT_ATT |= BIT_ATT;
	_delay_us(WAIT_DELAY);
}

static void avrshock2_sync(void)
{
	const uint8_t cmd = 0x42;
	for (uint8_t i = 0; i < 0x0C; ++i)
		avrshock2_cmd(&cmd, 1);
}

static void avrshock2_enter_cfg_mode(void)
{
	const uint8_t enter_cfg[3] = { 0x43, 0x00, 0x01 };
	avrshock2_cmd(enter_cfg, 3);
}

static void avrshock2_exit_cfg_mode(void)
{
	const uint8_t exit_cfg = 0x43;
	avrshock2_cmd(&exit_cfg, 1);
}


void avrshock2_init(void)
{
	DDR_DATA &= ~BIT_DATA;
	DDR_CMD |= BIT_CMD;
	DDR_ATT |= BIT_ATT;
	DDR_CLK |= BIT_CLK;

	PORT_ATT |= BIT_ATT;
	PORT_CMD |= BIT_CMD;

	avrshock2_sync();
}

void avrshock2_set_mode(const AVRSHOCK2_Mode mode, const bool lock)
{
	const uint8_t set_mode[4] = {
		0x44, 0x00,
		mode == AVRSHOCK2_MODE_DIGITAL ? 0x00 : 0x01,
		lock ? 0x03 : 0x00
	};

	do {
		avrshock2_enter_cfg_mode();
		avrshock2_cmd(set_mode, 4);
		avrshock2_exit_cfg_mode();
		avrshock2_sync();
	} while (avrshock2_currmode() != mode);
}

void avrshock2_poll(void)
{
	const uint8_t cmd = 0x42;
	avrshock2_cmd(&cmd, 1);

	memcpy(avrshock2_analogs, avrshock2_data_buffer + 5, 4);

	for (uint8_t i = AVRSHOCK2_BUTTON_FIRST; i <= AVRSHOCK2_BUTTON_LAST; ++i)
		avrshock2_buttons[i] = BUTTON_BIT(i) ? 0x00 : 0xFF;
}

