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
 * TODO:
 * for now only bit bang implementation is done, need to add 
 * implementation for AVR SPI hardware mode instead of software bit bang
 *
 * NOTE:
 * pin_data needs a pull-up resistor around 1K - 10K
 *
 * */


#define PS2C_RW_DELAY   (((1.0 / F_PS2C) * 1000000.0) / 2.0)
#define PS2C_WAIT_DELAY (16.2)

/* enum Button byte/bit index on ps2c_data_buffer */
#define PS2C_BUTTON_BYTE_INDEX(button) (3 + (button > 7))
#define PS2C_BUTTON_BIT_INDEX(button)  (1<<(button&0x07))


typedef uint8_t PinMode;
typedef uint8_t PinStat;

enum PinMode {
	OUTPUT,
	INPUT,
	INPUT_PULLUP
};

enum PinStat {
	HIGH,
	LOW
};

struct Pin {
	volatile uint8_t* const ddr;
	volatile uint8_t* const port;
	volatile uint8_t* const pin;
	const uint8_t bit;
};


uint8_t ps2c_buttons[PS2C_BUTTON_LAST + 1];
uint8_t ps2c_analogs[PS2C_ANALOG_LAST + 1];
uint8_t ps2c_data_buffer[36];

static const struct Pin pin_att  = { &DDRD, &PORTD, &PIND, 1<<PD2 };
static const struct Pin pin_cmd  = { &DDRD, &PORTD, &PIND, 1<<PD4 };
static const struct Pin pin_data = { &DDRD, &PORTD, &PIND, 1<<PD7 };
static const struct Pin pin_clk  = { &DDRB, &PORTB, &PINB, 1<<PB0 };


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

static void set_pin_mode(const struct Pin* const pin, const PinMode mode)
{
	if (mode == OUTPUT) {
		*pin->ddr |= pin->bit;
	} else {
		*pin->ddr &= ~pin->bit;
		write_pin(pin, mode == INPUT_PULLUP ? HIGH : LOW);	
	}
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

	write_pin(&pin_att, HIGH);
	_delay_us(PS2C_WAIT_DELAY);
}

static void ps2c_sync(void)
{
	const uint8_t cmd = 0x42;
	for (uint8_t i = 0; i < 0xFF; ++i)
		ps2c_cmd(&cmd, 1);
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


void ps2c_init(void)
{
	set_pin_mode(&pin_data, INPUT);
	set_pin_mode(&pin_cmd, OUTPUT);
	set_pin_mode(&pin_att, OUTPUT);
	set_pin_mode(&pin_clk, OUTPUT);
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
	} while (ps2c_data_buffer[1] != mode);
}

void ps2c_digital_poll(void)
{
	const uint8_t cmd = 0x42;
	ps2c_cmd(&cmd, 1);
	for (uint8_t i = PS2C_BUTTON_FIRST; i <= PS2C_BUTTON_LAST; ++i) {
		if (!(ps2c_data_buffer[PS2C_BUTTON_BYTE_INDEX(i)]&PS2C_BUTTON_BIT_INDEX(i)))
			ps2c_buttons[i] = 0xFF;
		else
			ps2c_buttons[i] = 0x00;
	}
}

void ps2c_analog_poll(void)
{
	const uint8_t cmd = 0x42;
	ps2c_cmd(&cmd, 1);

	/* get analog joys data */
	memcpy(ps2c_analogs, &ps2c_data_buffer[5], 4);

	if (ps2c_data_buffer[1] == PS2C_MODE_ANALOG_PRESSURE) {
		/* digital only buttons */
		const uint8_t d_order[4] = {
			PS2C_BUTTON_L3, PS2C_BUTTON_SELECT, PS2C_BUTTON_START, PS2C_BUTTON_R3
		};

		for (uint8_t i = 0; i < 4; ++i) {
			const uint8_t b = d_order[i];
			if (!(ps2c_data_buffer[PS2C_BUTTON_BYTE_INDEX(b)]&PS2C_BUTTON_BIT_INDEX(b)))
				ps2c_buttons[b] = 0xFF;
			else
				ps2c_buttons[b] = 0x00;
		}

		/* get pressure ones */
		const uint8_t p_order[12] = {
			PS2C_BUTTON_RIGHT, PS2C_BUTTON_LEFT, PS2C_BUTTON_UP, PS2C_BUTTON_DOWN,
			PS2C_BUTTON_TRI, PS2C_BUTTON_CIR, PS2C_BUTTON_X, PS2C_BUTTON_SQR,
			PS2C_BUTTON_L1, PS2C_BUTTON_R1, PS2C_BUTTON_L2, PS2C_BUTTON_R2
		};

		for (uint8_t i = 0; i < 12; ++i) {
			const uint8_t p = p_order[i];
			ps2c_buttons[p] = ps2c_data_buffer[9 + i];
		}
	} else {
		for (uint8_t i = PS2C_BUTTON_FIRST; i <= PS2C_BUTTON_LAST; ++i) {
			if (!(ps2c_data_buffer[PS2C_BUTTON_BYTE_INDEX(i)]&PS2C_BUTTON_BIT_INDEX(i)))
				ps2c_buttons[i] = 0xFF;
			else
				ps2c_buttons[i] = 0x00;
		}
	}
}

