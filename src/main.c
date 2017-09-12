#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "uart.h"


/* *
 *
 * By Rafael Moura 2017 (https://github.com/dhustkoder)
 *
 * TODO:
 * for now only bit bang implementation is done, need to add 
 * implementation for AVR SPI hardware mode instead of software bit bang
 *
 * NOTE:
 * pin_data needs a pull-up resistor around 1K - 10K
 *
 * */


#define PS2C_RW_DELAY   (((1.0 / F_PS2C) * 1000000.0) / 2.0)
#define PS2C_WAIT_DELAY (((1.0 / F_PS2C) * 1000000.0) * 10.0)

/* enum Buttons byte/bit index on data_buffer */
#define BUTTON_BYTE_INDEX(button) (3 + (button > 7))
#define BUTTON_BIT_INDEX(button)  (1<<(button&0x07))

typedef uint8_t PinMode;
typedef uint8_t PinStat;
typedef uint8_t PS2C_Mode;

enum PinMode {
	OUTPUT,
	INPUT,
	INPUT_PULLUP
};

enum PinStat {
	HIGH,
	LOW
};

enum PS2C_Mode {
	PS2C_MODE_DIGITAL         = 0x41,
	PS2C_MODE_ANALOG          = 0x73,
	PS2C_MODE_ANALOG_PRESSURE = 0x79,
	PS2C_MODE_CONFIG          = 0xF3
};

/* Buttons enums are in byte/bit order as they come in data_buffer[3]...[4] */
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

/* AnalogJoys enums are in byte order as they come in data_buffer[5]...[9] */
enum AnalogJoys {
	ANALOG_JOY_RX,
	ANALOG_JOY_RY,
	ANALOG_JOY_LX,
	ANALOG_JOY_LY,
	ANALOG_JOY_FIRST = ANALOG_JOY_RX,
	ANALOG_JOY_LAST  = ANALOG_JOY_LY
};


struct Pin {
	volatile uint8_t* const ddr;
	volatile uint8_t* const port;
	volatile uint8_t* const pin;
	const uint8_t bit;
};


static const struct Pin pin_att  = { &DDRD, &PORTD, &PIND, 1<<PD2 };
static const struct Pin pin_cmd  = { &DDRD, &PORTD, &PIND, 1<<PD4 };
static const struct Pin pin_data = { &DDRD, &PORTD, &PIND, 1<<PD7 };
static const struct Pin pin_clk  = { &DDRB, &PORTB, &PINB, 1<<PB0 };

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


static void set_pin_mode(const struct Pin* pin, PinMode mode);
static void write_pin(const struct Pin* pin, PinStat stat);
static PinStat read_pin(const struct Pin* pin);
static void ps2c_cmd(const uint8_t* restrict cmd, uint8_t cmdsize);
static void ps2c_sync(void);


static void set_pin_mode(const struct Pin* const pin, const PinMode mode)
{
	if (mode == OUTPUT) {
		*pin->ddr |= pin->bit;
	} else {
		*pin->ddr &= ~pin->bit;
		write_pin(pin, mode == INPUT_PULLUP ? HIGH : LOW);	
	}
}

static void write_pin(const struct Pin* const pin, const PinStat stat)
{
	if (stat == HIGH)
		*pin->port |= pin->bit;
	else
		*pin->port &= ~pin->bit;
}

static PinStat read_pin(const struct Pin* const pin)
{
	return ((*pin->pin)&pin->bit) ? HIGH : LOW;
}

static void ps2c_init(void)
{
	set_pin_mode(&pin_data, INPUT);
	set_pin_mode(&pin_cmd, OUTPUT);
	set_pin_mode(&pin_att, OUTPUT);
	set_pin_mode(&pin_clk, OUTPUT);
	ps2c_sync();
}

static uint8_t ps2c_exchange(const uint8_t out) 
{
	uint8_t in = 0x00;
	for (unsigned b = 0; b < 8; ++b) {
		write_pin(&pin_cmd, (out&(0x01<<b)) ? HIGH : LOW);

		write_pin(&pin_clk, LOW);
		_delay_us(PS2C_RW_DELAY);

		if (read_pin(&pin_data) == HIGH)
			in |= (0x01<<b);

		write_pin(&pin_clk, HIGH);
		_delay_us(PS2C_RW_DELAY);
	}

	write_pin(&pin_cmd, HIGH);
	_delay_us(PS2C_WAIT_DELAY);

	return in;
}

static void ps2c_cmd(const uint8_t* const restrict cmd, const uint8_t cmdsize)
{
	write_pin(&pin_att, LOW);
	_delay_us(PS2C_WAIT_DELAY);

	/* send first 2 */
	data_buffer[0] = ps2c_exchange(0x01);   /* the first byte in header is always 0x01 */
	data_buffer[1] = ps2c_exchange(cmd[0]); /* send second byte and get current mode   */
	/* calculate the total number of bytes to send and receive based on the mode */
	const uint8_t recvsize = ((data_buffer[1]&0x0F) * 2) + 3;

	/* send and receive the rest of the data */
	for (uint8_t i = 2; i < recvsize; ++i) {
		const uint8_t out = i <= cmdsize ? cmd[i - 1] : 0x00;
		data_buffer[i] = ps2c_exchange(out);
	}

	write_pin(&pin_att, HIGH);
	_delay_us(PS2C_WAIT_DELAY);
}

static void ps2c_sync(void)
{
	const uint8_t cmd = 0x42;
	for (uint8_t i = 0; i < 0xFF; ++i)
		ps2c_cmd(&cmd, 1);
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
	const uint8_t cmd = 0x42;
	ps2c_cmd(&cmd, 1);

	/* get analog joys data */
	memcpy(analog_joys, &data_buffer[5], 4);

	if (data_buffer[1] == PS2C_MODE_ANALOG_PRESSURE) {
		/* digital only buttons */
		const uint8_t d_order[4] = {
			BUTTON_L3, BUTTON_SELECT, BUTTON_START, BUTTON_R3
		};

		for (uint8_t i = 0; i < 4; ++i) {
			const uint8_t b = d_order[i];
			if (!(data_buffer[BUTTON_BYTE_INDEX(b)]&BUTTON_BIT_INDEX(b)))
				button_state[b] = 0xFF;
			else
				button_state[b] = 0x00;
		}

		/* get pressure ones */
		const uint8_t p_order[12] = {
			BUTTON_RIGHT, BUTTON_LEFT, BUTTON_UP, BUTTON_DOWN,
			BUTTON_TRI, BUTTON_CIR, BUTTON_X, BUTTON_SQR,
			BUTTON_L1, BUTTON_R1, BUTTON_L2, BUTTON_R2
		};

		for (uint8_t i = 0; i < 12; ++i) {
			const uint8_t p = p_order[i];
			button_state[p] = data_buffer[9 + i];
		}
	} else {
		for (uint8_t i = BUTTON_FIRST; i <= BUTTON_LAST; ++i) {
			if (!(data_buffer[BUTTON_BYTE_INDEX(i)]&BUTTON_BIT_INDEX(i)))
				button_state[i] = 0xFF;
			else
				button_state[i] = 0x00;
		}
	}
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

static void ps2c_set_mode(const PS2C_Mode mode, const bool lock)
{
	const uint8_t set_mode[4] = {
		0x44, 0x00,
		mode == PS2C_MODE_DIGITAL ? 0x00 : 0x01,
		lock ? 0x03 : 0x00
	};
	
	do {
		ps2c_enter_cfg_mode();
		ps2c_cmd(set_mode, 4);
		
		if (mode == PS2C_MODE_ANALOG_PRESSURE) {
			uint8_t init_pressure[4] = {
				0x40, 0x00, 0x00, 0x02
			};

			const uint8_t cfg_pressure[5] = {
				0x4F, 0x00, 0xFF, 0xFF, 0x03
			};

			for (uint8_t i = 0; i < 0x0C; ++i) {
				/* enable pressure 0x00 - 0x0B */
				init_pressure[2] = i;
				ps2c_cmd(init_pressure, 4);
				ps2c_cmd(cfg_pressure, 5);
			}
		}

		ps2c_exit_cfg_mode();
		ps2c_sync();
	} while (data_buffer[1] != mode);
}


__attribute__((noreturn)) void main(void)
{
	ps2c_init();
	uart_init();

	/* let's play with LEDs with analog stick */
	const struct Pin pins[] = {
		{ &DDRD, &PORTD, &PIND, 1<<PD3 },
		{ &DDRD, &PORTD, &PIND, 1<<PD5 },
		{ &DDRD, &PORTD, &PIND, 1<<PD6 },
		{ &DDRB, &PORTB, &PINB, 1<<PB1 }
	};
	const uint8_t joyorder[] = {
		ANALOG_JOY_LX, ANALOG_JOY_LX,
		ANALOG_JOY_LY, ANALOG_JOY_LY
	};

	for (uint8_t i = 0; i < 4; ++i)
		set_pin_mode(&pins[i], OUTPUT);

	ps2c_set_mode(PS2C_MODE_ANALOG_PRESSURE, true);

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

		uint8_t toggle = 0xFF;
		for (uint8_t i = 0; i < 4; ++i) {
			write_pin(&pins[i], analog_joys[joyorder[i]] == toggle ? HIGH : LOW);
			toggle ^= 0xFF;
		};
	}
}

