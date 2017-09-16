#include <stdbool.h>
#include <string.h>
#include <stdnoreturn.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "ps2c.h"

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

#ifndef F_PS2C
#define F_PS2C 250000UL
#endif

#define RW_DELAY   (((1.0 / F_PS2C) * 1000000.0) / 2.0)
#define WAIT_DELAY (10.0)

/* enum Button byte/bit index on ps2c_data_buffer */
#define BUTTON_BYTE(button) ps2c_data_buffer[(3 + (button > 7))]
#define BUTTON_BIT(button)  BUTTON_BYTE(button)&(1<<(button&0x07))


uint8_t ps2c_buttons[PS2C_BUTTON_LAST + 1];
uint8_t ps2c_analogs[PS2C_ANALOG_LAST + 1];
uint8_t ps2c_data_buffer[34];


static uint8_t ps2c_exchange(const uint8_t out)
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

static void ps2c_cmd(const uint8_t* const restrict cmd, const uint8_t cmdsize)
{
	PORT_ATT &= ~BIT_ATT;
	_delay_us(WAIT_DELAY);

	/* send first 2 */
	ps2c_data_buffer[0] = ps2c_exchange(0x01);   /* the first byte in header is always 0x01 */
	ps2c_data_buffer[1] = ps2c_exchange(cmd[0]); /* send second byte and get current mode   */
	/* calculate the total number of bytes to send and receive based on the mode */
	const uint8_t recvsize = ((ps2c_data_buffer[1]&0x0F) * 2) + 3;

	/* send and receive the rest of the data */
	uint8_t i = 2;
	for (; i <= cmdsize; ++i)
		ps2c_data_buffer[i] = ps2c_exchange(cmd[i - 1]);
	for (; i < recvsize; ++i)
		ps2c_data_buffer[i] = ps2c_exchange(0x00);

	PORT_ATT |= BIT_ATT;
	_delay_us(WAIT_DELAY);
}

static void ps2c_sync(void)
{
	const uint8_t cmd = 0x42;
	for (uint8_t i = 0; i < 0x0C; ++i)
		ps2c_cmd(&cmd, 1);
}

static void ps2c_enter_cfg_mode(void)
{
	const uint8_t enter_cfg[3] = { 0x43, 0x00, 0x01 };
	ps2c_cmd(enter_cfg, 3);
}

static void ps2c_exit_cfg_mode(void)
{
	const uint8_t exit_cfg = 0x43;
	ps2c_cmd(&exit_cfg, 1);
}


void ps2c_init(void)
{
	DDR_DATA &= ~BIT_DATA;
	DDR_CMD |= BIT_CMD;
	DDR_ATT |= BIT_ATT;
	DDR_CLK |= BIT_CLK;

	PORT_ATT |= BIT_ATT;
	PORT_CMD |= BIT_CMD;

	ps2c_sync();
}

void ps2c_set_mode(const PS2C_Mode mode, const bool lock)
{
	const uint8_t set_mode[4] = {
		0x44, 0x00,
		mode == PS2C_MODE_DIGITAL ? 0x00 : 0x01,
		lock ? 0x03 : 0x00
	};

	do {
		ps2c_enter_cfg_mode();
		ps2c_cmd(set_mode, 4);
		ps2c_exit_cfg_mode();
		ps2c_sync();
	} while (ps2c_currmode() != mode);
}

void ps2c_poll(void)
{
	const uint8_t cmd = 0x42;
	ps2c_cmd(&cmd, 1);

	memcpy(ps2c_analogs, ps2c_data_buffer + 5, 4);

	for (uint8_t i = PS2C_BUTTON_FIRST; i <= PS2C_BUTTON_LAST; ++i)
		ps2c_buttons[i] = BUTTON_BIT(i) ? 0x00 : 0xFF;
}

