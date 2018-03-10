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
 * TODO: (pressure and motor support)
 * Couldn't properly control pressure mode, need more information about 
 * the controller commands, and need add vibration motor controll
 *
 * NOTE:
 * PIN_DAT needs a pull-up resistor around 1K - 10K
 *
 * */

#if !defined(AVRSHOCK2_PORT_ATT) || !defined(AVRSHOCK2_DDR_ATT) || !defined(AVRSHOCK2_BIT_ATT) ||                                 \
    !defined(AVRSHOCK2_PORT_CMD) || !defined(AVRSHOCK2_DDR_CMD) || !defined(AVRSHOCK2_BIT_CMD) ||                                 \
    !defined(AVRSHOCK2_PORT_DAT) || !defined(AVRSHOCK2_DDR_DAT) || !defined(AVRSHOCK2_PIN_DAT) || !defined(AVRSHOCK2_BIT_DAT) ||  \
    !defined(AVRSHOCK2_PORT_CLK) || !defined(AVRSHOCK2_DDR_CLK) || !defined(AVRSHOCK2_BIT_CLK)
#error Need all AVRSHOCK2 Pins definitions.
#elif !defined(AVRSHOCK2_BIT_BANG) && !defined(AVRSHOCK2_SPI)
#error At least one mode should be defined.
#endif

#define PORT_ATT  AVRSHOCK2_PORT_ATT
#define DDR_ATT   AVRSHOCK2_DDR_ATT 
#define BIT_ATT   (0x01<<AVRSHOCK2_BIT_ATT)

#define PORT_CMD  AVRSHOCK2_PORT_CMD
#define DDR_CMD   AVRSHOCK2_DDR_CMD 
#define BIT_CMD   (0x01<<AVRSHOCK2_BIT_CMD)

#define PORT_DAT  AVRSHOCK2_PORT_DAT
#define DDR_DAT   AVRSHOCK2_DDR_DAT 
#define PIN_DAT   AVRSHOCK2_PIN_DAT 
#define BIT_DAT   (0x01<<AVRSHOCK2_BIT_DAT)

#define PORT_CLK  AVRSHOCK2_PORT_CLK 
#define DDR_CLK   AVRSHOCK2_DDR_CLK  
#define BIT_CLK   (0x01<<AVRSHOCK2_BIT_CLK)

#ifndef F_AVRSHOCK2
#define F_AVRSHOCK2 250000UL
#endif

#define CLK_DELAY      (((1.0 / F_AVRSHOCK2) * 1000000.0) / 2.0)
#define ATT_DELAY      (2)
#define EXCHANGE_DELAY (6)


uint8_t avrshock2_data_buffer[33];


static uint8_t exchange(const uint8_t out)
{
	_delay_us(EXCHANGE_DELAY);

	#ifdef AVRSHOCK2_BIT_BANG
	uint8_t in = 0x00;
	for (uint8_t b = 0; b < 8; ++b) {
		if (out&(0x01<<b))
			PORT_CMD |= BIT_CMD;
		else
			PORT_CMD &= ~BIT_CMD;

		PORT_CLK &= ~BIT_CLK;
		_delay_us(CLK_DELAY);

		if (PIN_DAT&BIT_DAT)
			in |= (0x01<<b);

		PORT_CLK |= BIT_CLK;
		_delay_us(CLK_DELAY);
	}

	PORT_CMD |= BIT_CMD;
	return in;

	#else

	SPDR = out;
	while (!(SPSR&(0x01<<SPIF)))
		_delay_us(1);
	return SPDR;

	#endif
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
	DDR_DAT &= ~BIT_DAT;
	DDR_CMD |= BIT_CMD;
	DDR_ATT |= BIT_ATT;
	DDR_CLK |= BIT_CLK;

	PORT_ATT |= BIT_ATT;
	PORT_CMD |= BIT_CMD;

	#ifdef AVRSHOCK2_BIT_BANG
	SPCR &= ~(0x01<<SPE);
	#else
	#if F_AVRSHOCK2 != 250000
	#error AVRSHOCK2 SPI Mode only support frequency 250000
	#endif
	/* set SPI control register */
	SPCR = (0x01<<SPR1)|(0x01<<CPHA)|(0x01<<CPOL)|(0x01<<MSTR)|(0x01<<DORD)|(0x01<<SPE);
	/* set SPI status register  */
	SPSR &= ~SPI2X; /* set SPI frequency */
	#endif

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
                    avrshock2_axis_t axis[AVRSHOCK2_AXIS_NAXIS])
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
		memcpy(axis, &data[2], AVRSHOCK2_AXIS_NAXIS);

	old_mode = mode;
	memcpy(old_data, data, data_size);
	return true;
}

