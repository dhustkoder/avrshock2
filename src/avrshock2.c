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
#define F_AVRSHOCK2 496000UL
#endif

#define CLK_DELAY      (((1.0 / F_AVRSHOCK2) * 1000000.0) / 2.0)
#define ATT_DELAY      (2)
#define EXCHANGE_DELAY (6)


uint8_t avrshock2_data_buffer[33];


static uint8_t exchange(const uint8_t out)
{
	_delay_us(EXCHANGE_DELAY);
	uint8_t in = 0x00;
	for (uint8_t b = 0; b < 8; ++b) {
		if (out&(0x01<<b))
			PORT_CMD |= BIT_CMD;
		else
			PORT_CMD &= ~BIT_CMD;

		PORT_CLK &= ~BIT_CLK;
		_delay_us(CLK_DELAY);

		if (PIN_DATA&BIT_DATA)
			in |= (0x01<<b);

		PORT_CLK |= BIT_CLK;
		_delay_us(CLK_DELAY);
	}

	PORT_CMD |= BIT_CMD;
	return in;
}

static void send_cmd(const uint8_t* const restrict cmd, const uint8_t cmdsize)
{
	PORT_ATT &= ~BIT_ATT;
	_delay_us(ATT_DELAY);

	/* send first 2 */
	avrshock2_data_buffer[0] = exchange(0x01);   /* the first byte in header is always 0x01 */
	avrshock2_data_buffer[1] = exchange(cmd[0]); /* send second byte and get current mode   */
	/* calculate the total number of bytes to send and receive based on the mode */
	const uint8_t recvsize = ((avrshock2_data_buffer[1]&0x0F) * 2) + 3;

	/* send and receive the rest of the data */
	short i = 2;
	for (; i <= cmdsize; ++i)
		avrshock2_data_buffer[i] = exchange(cmd[i - 1]);
	for (; i < recvsize; ++i)
		avrshock2_data_buffer[i] = exchange(0x00);

	PORT_ATT |= BIT_ATT;
	_delay_us(ATT_DELAY);
}

static void poll(const short times)
{
	const uint8_t cmd = 0x42;
	for (int i = 0; i < times; ++i)
		send_cmd(&cmd, 1);
}

static void enter_cfg_mode(void)
{
	const uint8_t enter_cfg[3] = { 0x43, 0x00, 0x01 };
	send_cmd(enter_cfg, 3);
}

static void exit_cfg_mode(void)
{
	const uint8_t exit_cfg = 0x43;
	send_cmd(&exit_cfg, 1);
}


void avrshock2_init(void)
{
	DDR_DATA &= ~BIT_DATA;
	DDR_CMD |= BIT_CMD;
	DDR_ATT |= BIT_ATT;
	DDR_CLK |= BIT_CLK;

	PORT_ATT |= BIT_ATT;
	PORT_CMD |= BIT_CMD;

	poll(16);
}

void avrshock2_set_mode(const avrshock2_mode_t mode, const bool lock)
{
	const uint8_t mode_cmd[4] = {
		0x44, 0x00,
		mode == AVRSHOCK2_MODE_DIGITAL ? 0x00 : 0x01,
		lock ? 0x03 : 0x00
	};

	do {
		enter_cfg_mode();
		send_cmd(mode_cmd, 4);
		exit_cfg_mode();
		poll(64);
	} while (mode != avrshock2_get_mode());
}

bool avrshock2_poll(avrshock2_button_t* const buttons,
                    avrshock2_axis_t axis[AVRSHOCK2_AXIS_SIZE])
{
	static avrshock2_mode_t old_mode = 0;
	static uint8_t old_data[6] = { 0 };

	poll(1);

	const uint8_t* const data = &avrshock2_data_buffer[3];
	const avrshock2_mode_t mode = avrshock2_get_mode();
	const uint8_t data_size = mode == AVRSHOCK2_MODE_DIGITAL ? 2 : 6;
	
	if (old_mode == mode && memcmp(old_data, data, data_size) == 0)
		return false;

	*buttons = ~((data[1]<<8)|data[0]);
	if (mode == AVRSHOCK2_MODE_ANALOG)
		memcpy(axis, &data[2], AVRSHOCK2_AXIS_SIZE);

	old_mode = mode;
	memcpy(old_data, data, data_size);
	return true;
}

