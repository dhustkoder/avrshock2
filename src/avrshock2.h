#ifndef AVRSHOCK2_H_
#define AVRSHOCK2_H_
#include <stdbool.h>
#include <stdint.h>


typedef uint8_t AVRSHOCK2_Mode;
enum AVRSHOCK2_Mode {
	AVRSHOCK2_MODE_DIGITAL = 0x41,
	AVRSHOCK2_MODE_ANALOG  = 0x73
};

/* Button enums are in byte/bit order as they come in data_buffer[3]...[4] */
typedef uint8_t AVRSHOCK2_Button;
enum AVRSHOCK2_Button {
	AVRSHOCK2_BUTTON_SELECT = 0x00,
	AVRSHOCK2_BUTTON_L3     = 0x01,
	AVRSHOCK2_BUTTON_R3     = 0x02,
	AVRSHOCK2_BUTTON_START  = 0x03,
	AVRSHOCK2_BUTTON_UP     = 0x04,
	AVRSHOCK2_BUTTON_RIGHT  = 0x05,
	AVRSHOCK2_BUTTON_DOWN   = 0x06,
	AVRSHOCK2_BUTTON_LEFT   = 0x07,

	AVRSHOCK2_BUTTON_L2     = 0x08,
	AVRSHOCK2_BUTTON_R2     = 0x09,
	AVRSHOCK2_BUTTON_L1     = 0x0A,
	AVRSHOCK2_BUTTON_R1     = 0x0B,
	AVRSHOCK2_BUTTON_TRI    = 0x0C,
	AVRSHOCK2_BUTTON_CIRCLE = 0x0D,
	AVRSHOCK2_BUTTON_CROSS  = 0x0E,
	AVRSHOCK2_BUTTON_SQUARE = 0x0F,
	AVRSHOCK2_BUTTON_FIRST  = AVRSHOCK2_BUTTON_SELECT,
	AVRSHOCK2_BUTTON_LAST   = AVRSHOCK2_BUTTON_SQUARE
};

/* AVRSHOCK2_Analog enums are in byte order as they come in data_buffer[5]...[9] */
typedef uint8_t AVRSHOCK2_Analog;
enum AVRSHOCK2_Analog {
	AVRSHOCK2_ANALOG_RX,
	AVRSHOCK2_ANALOG_RY,
	AVRSHOCK2_ANALOG_LX,
	AVRSHOCK2_ANALOG_LY,
	AVRSHOCK2_ANALOG_FIRST = AVRSHOCK2_ANALOG_RX,
	AVRSHOCK2_ANALOG_LAST  = AVRSHOCK2_ANALOG_LY
};


extern uint8_t avrshock2_buttons[AVRSHOCK2_BUTTON_LAST + 1]; /* 0x00 released - 0xFF fully pressed */
extern uint8_t avrshock2_analogs[AVRSHOCK2_ANALOG_LAST + 1]; /* 0x7F - middle point                */

void avrshock2_init(void);
void avrshock2_set_mode(AVRSHOCK2_Mode mode, bool lock);
void avrshock2_poll(void);


static inline AVRSHOCK2_Mode avrshock2_currmode(void)
{
	extern uint8_t avrshock2_data_buffer[34];
	return avrshock2_data_buffer[1];
}


#endif

