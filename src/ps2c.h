#ifndef PS2C_PS2C_H_
#define PS2C_PS2C_H_
#include <stdbool.h>
#include <stdint.h>


typedef uint8_t PS2C_Mode;
typedef uint8_t PS2C_Button;
typedef uint8_t PS2C_Analog;

enum PS2C_Mode {
	PS2C_MODE_DIGITAL = 0x41,
	PS2C_MODE_ANALOG  = 0x73
};

/* Button enums are in byte/bit order as they come in data_buffer[3]...[4] */
enum PS2C_Button {
	PS2C_BUTTON_SELECT = 0x00,
	PS2C_BUTTON_L3     = 0x01,
	PS2C_BUTTON_R3     = 0x02,
	PS2C_BUTTON_START  = 0x03,
	PS2C_BUTTON_UP     = 0x04,
	PS2C_BUTTON_RIGHT  = 0x05,
	PS2C_BUTTON_DOWN   = 0x06,
	PS2C_BUTTON_LEFT   = 0x07,

	PS2C_BUTTON_L2     = 0x08,
	PS2C_BUTTON_R2     = 0x09,
	PS2C_BUTTON_L1     = 0x0A,
	PS2C_BUTTON_R1     = 0x0B,
	PS2C_BUTTON_TRI    = 0x0C,
	PS2C_BUTTON_CIR    = 0x0D,
	PS2C_BUTTON_X      = 0x0E,
	PS2C_BUTTON_SQR    = 0x0F,
	PS2C_BUTTON_FIRST  = PS2C_BUTTON_SELECT,
	PS2C_BUTTON_LAST   = PS2C_BUTTON_SQR
};

/* PS2C_Analog enums are in byte order as they come in data_buffer[5]...[9] */
enum PS2C_Analog {
	PS2C_ANALOG_RX,
	PS2C_ANALOG_RY,
	PS2C_ANALOG_LX,
	PS2C_ANALOG_LY,
	PS2C_ANALOG_FIRST = PS2C_ANALOG_RX,
	PS2C_ANALOG_LAST  = PS2C_ANALOG_LY
};


static inline PS2C_Mode ps2c_currmode(void)
{
	extern uint8_t ps2c_data_buffer[34];
	return ps2c_data_buffer[1];
}


void ps2c_init(void);
void ps2c_set_mode(const PS2C_Mode mode, const bool lock);
void ps2c_poll(void);

extern uint8_t ps2c_buttons[PS2C_BUTTON_LAST + 1]; /* 0x00 released - 0xFF fully pressed */
extern uint8_t ps2c_analogs[PS2C_ANALOG_LAST + 1]; /* 0x7F - middle point                */


#endif

