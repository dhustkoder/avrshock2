#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "uart.h"


/* *
 * By Rafael Moura 2017 (https://github.com/dhustkoder)
 *
 * TODO:
 * for now only bit bang implementation is done, need to add 
 * implementation for AVR SPI hardware mode instead of software bit bang
 *
 * NOTE:
 * PORT_DATA needs a pull-up resistor around 1K - 10K
 *
 * */


#define PS2C_RW_DELAY   (((1.0 / F_PS2C) * 1000000.0) / 2.0)
#define PS2C_WAIT_DELAY (((1.0 / F_PS2C) * 1000000.0) * 5.0)

#define PORT_MODE DDRB
#define PORT_STATUS PORTB
#define PIN_STATUS PINB
#define BUTTON_BYTE_INDEX(button) (3 + (button > 7))
#define BUTTON_BIT_INDEX(button)  (1<<(button&0x07))

enum Ports {
	/* SPI */
	PORT_MOSI = _BV(PB3),
	PORT_MISO = _BV(PB4),
	PORT_SCK = _BV(PB5),
	PORT_SS = _BV(PB2),

	/* PS2 controller */
	PORT_COMMAND =  PORT_MOSI,
	PORT_DATA = PORT_MISO,
	PORT_CLOCK = PORT_SCK,
	PORT_ATTENTION = PORT_SS
};

enum Pins {
	/* SPI */
	PIN_MOSI = _BV(PINB3),
	PIN_MISO = _BV(PINB4),
	PIN_SCK = _BV(PINB5),
	PIN_SS = _BV(PINB2),

	/* PS2 controller */
	PIN_COMMAND = PIN_MOSI,
	PIN_DATA = PIN_MISO,
	PIN_CLOCK = PIN_SCK,
	PIN_ATTENTION = PIN_SS
};

enum PS2C_Mode {
	PS2C_MODE_DIGITAL = 0x40,
	PS2C_MODE_ANALOG  = 0x70,
	PS2C_MODE_CONFIG  = 0xF0
};

enum Buttons {
	BUTTON_SELECT = 0x00,
	BUTTON_L3     = 0x01,
	BUTTON_R3     = 0x02,
	BUTTON_START  = 0x03,
	BUTTON_UP     = 0x04,
	BUTTON_RIGHT  = 0x05,
	BUTTON_DOWN   = 0x06,
	BUTTON_LEFT   = 0x07,

	BUTTON_L2     = 0x08,
	BUTTON_R2     = 0x09,
	BUTTON_L1     = 0x0A,
	BUTTON_R1     = 0x0B,
	BUTTON_TRI    = 0x0C,
	BUTTON_CIR    = 0x0D,
	BUTTON_X      = 0x0E,
	BUTTON_SQR    = 0x0F,
	BUTTON_FIRST  = BUTTON_SELECT,
	BUTTON_LAST   = BUTTON_SQR
};

enum AnalogJoys {
	ANALOG_JOY_RX,
	ANALOG_JOY_RY,
	ANALOG_JOY_LX,
	ANALOG_JOY_LY,
	ANALOG_JOY_FIRST = ANALOG_JOY_RX,
	ANALOG_JOY_LAST  = ANALOG_JOY_LY
};

static const char* const button_name[] = {
	[BUTTON_SELECT] = "SELECT",
	[BUTTON_L3]     = "L3",
	[BUTTON_R3]     = "R3",
	[BUTTON_START]  = "START",
	[BUTTON_UP]     = "UP",
	[BUTTON_RIGHT]  = "RIGHT",
	[BUTTON_DOWN]   = "DOWN",
	[BUTTON_LEFT]   = "LEFT",
	[BUTTON_L2]     = "L2",
	[BUTTON_R2]     = "R2",
	[BUTTON_L1]     = "L1",
	[BUTTON_R1]     = "R1",
	[BUTTON_TRI]    = "TRIANGLE",
	[BUTTON_CIR]    = "CIRCLE",
	[BUTTON_X]      = "X",
	[BUTTON_SQR]    = "SQUARE"
};

static const char* const analog_joy_name[] = {
	[ANALOG_JOY_RX] = "RX",
	[ANALOG_JOY_RY] = "RY",
	[ANALOG_JOY_LX] = "LX",
	[ANALOG_JOY_LY] = "LY"
};

static uint8_t button_state[BUTTON_LAST + 1];
static uint8_t analog_joys[ANALOG_JOY_LAST + 1];
static uint8_t data_buffer[36];


static void ps2c_cmd(const uint8_t* restrict cmd, uint8_t sendsize);


static void ps2c_init(void)
{
	PORT_MODE &= ~PORT_DATA; /* DATA input */
	PORT_MODE |= PORT_COMMAND|PORT_ATTENTION|PORT_CLOCK; /* others out */
	PORT_STATUS |= PORT_ATTENTION;

	/* sync connection */
	const uint8_t n = 0x42;
	for (uint8_t i = 0; i < 10; ++i)
		ps2c_cmd(&n, 1);
}

static uint8_t ps2c_exchange(const uint8_t out) 
{
	uint8_t in = 0x00;
	for (unsigned b = 0; b < 8; ++b) {
		if (out&(0x01<<b))
			PORT_STATUS |= PORT_COMMAND;
		else
			PORT_STATUS &= ~PORT_COMMAND;

		PORT_STATUS &= ~PORT_CLOCK;
		_delay_us(PS2C_RW_DELAY);

		if (PIN_STATUS&PIN_DATA)
			in |= (0x01<<b);

		PORT_STATUS |= PORT_CLOCK;
		_delay_us(PS2C_RW_DELAY);
	}

	PORT_STATUS |= PORT_COMMAND;
	_delay_us(PS2C_WAIT_DELAY);

	return in;
}

static void ps2c_cmd(const uint8_t* const restrict cmd, const uint8_t sendsize)
{
	PORT_STATUS &= ~PORT_ATTENTION;
	_delay_us(PS2C_WAIT_DELAY);

	/* send first 2 */
	data_buffer[0] = ps2c_exchange(0x01); /* ignore first byte in */
	data_buffer[1] = ps2c_exchange(cmd[0]); /* send second byte and get current mode */
	const uint8_t recvsize = ((data_buffer[1]&0x0F) * 2) + 3;

	for (uint8_t i = 2; i < recvsize; ++i) {
		const uint8_t out = i <= sendsize ? cmd[i - 1] : 0x00;
		data_buffer[i] = ps2c_exchange(out);
	}

	PORT_STATUS |= PORT_ATTENTION;
	_delay_us(PS2C_WAIT_DELAY);
}

static void ps2c_digital_poll(void)
{
	const uint8_t cmd = 0x42;
	ps2c_cmd(&cmd, 1);
	for (uint8_t i = BUTTON_FIRST; i <= BUTTON_LAST; ++i) {
		if (!(data_buffer[BUTTON_BYTE_INDEX(i)]&BUTTON_BIT_INDEX(i)))
			button_state[i] = 0xFF;
		else
			button_state[i] = 0x00;
	}
}

static void ps2c_analog_poll(void)
{
	/* TODO: get pressure working */

	/* takes care of digital buttons (no pressure) */
	ps2c_digital_poll();

	/* get analog joys data */
	memcpy(analog_joys, &data_buffer[5], 4);
}

static void ps2c_enter_cfg_mode(void)
{
	const uint8_t enter_cfg[3] = { 0x43, 0x00, 0x01 };
	ps2c_cmd(enter_cfg, 3);
}

static void ps2c_exit_cfg_mode(void)
{
	const uint8_t exit_cfg[8] = {
		0x43, 0x00, 0x00, 0x5A,
		0x5A, 0x5A, 0x5A, 0x5A
	};
	ps2c_cmd(exit_cfg, 8);
}

static void ps2c_set_mode(const enum PS2C_Mode mode, const bool lock)
{
	const uint8_t set_mode[4] = {
		0x44, 0x00,
		mode == PS2C_MODE_DIGITAL ? 0x00 : 0x01,
		lock ? 0x03 : 0x00
	};

	const uint8_t motor_mapping[8] = {
		0x4D, 0x00, 0x00, 0x01,
		0xFF, 0xFF, 0xFF, 0xFF
	};

	const uint8_t cfg_pressure[5] = {
		0x4F, 0x00, 0xFF, 0xFF, 0x03
	};

	ps2c_enter_cfg_mode();

	ps2c_cmd(set_mode, 4);
	ps2c_cmd(motor_mapping, 8);
	ps2c_cmd(cfg_pressure, 5);

	ps2c_exit_cfg_mode();
}


__attribute__((noreturn)) void main(void)
{
	ps2c_init();
	uart_init();

	ps2c_set_mode(PS2C_MODE_ANALOG, true);

	for (;;) {
		ps2c_analog_poll();

		putchar(12);

		printf("\n\nMODE: %.2X\n", data_buffer[1]);
		printf("ANALOG JOYS [%.3d %.3d %.3d %.3d]",
		       analog_joys[0], analog_joys[1],
		       analog_joys[2], analog_joys[3]);

		printf("\n");
		
		for (uint8_t i = BUTTON_FIRST; i <= BUTTON_LAST; ++i)
			printf(" %.3d ", button_state[i]);

		_delay_ms(500);
	}
}

