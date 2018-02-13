#ifndef AVRSHOCK2_H_
#define AVRSHOCK2_H_
#include <stdbool.h>
#include <stdint.h>


typedef uint8_t avrshock2_mode_t;
#define AVRSHOCK2_MODE_DIGITAL ((avrshock2_mode_t)0x41)
#define AVRSHOCK2_MODE_ANALOG  ((avrshock2_mode_t)0x73)

/* avrshock2_button_t are in bit order as they come in data buffer [3]...[4] */
typedef uint16_t avrshock2_button_t;
#define AVRSHOCK2_BUTTON_SELECT   ((avrshock2_button_t)0x0001)
#define AVRSHOCK2_BUTTON_L3       ((avrshock2_button_t)0x0002)
#define AVRSHOCK2_BUTTON_R3       ((avrshock2_button_t)0x0004)
#define AVRSHOCK2_BUTTON_START    ((avrshock2_button_t)0x0008)
#define AVRSHOCK2_BUTTON_UP       ((avrshock2_button_t)0x0010)
#define AVRSHOCK2_BUTTON_RIGHT    ((avrshock2_button_t)0x0020)
#define AVRSHOCK2_BUTTON_DOWN     ((avrshock2_button_t)0x0040)
#define AVRSHOCK2_BUTTON_LEFT     ((avrshock2_button_t)0x0080)
#define AVRSHOCK2_BUTTON_L2       ((avrshock2_button_t)0x0100)
#define AVRSHOCK2_BUTTON_R2       ((avrshock2_button_t)0x0200)
#define AVRSHOCK2_BUTTON_L1       ((avrshock2_button_t)0x0400)
#define AVRSHOCK2_BUTTON_R1       ((avrshock2_button_t)0x0800)
#define AVRSHOCK2_BUTTON_TRIANGLE ((avrshock2_button_t)0x1000)
#define AVRSHOCK2_BUTTON_CIRCLE   ((avrshock2_button_t)0x2000)
#define AVRSHOCK2_BUTTON_CROSS    ((avrshock2_button_t)0x4000)
#define AVRSHOCK2_BUTTON_SQUARE   ((avrshock2_button_t)0x8000)
#define AVRSHOCK2_BUTTON_NBUTTONS (16)

/* avrshock2_axis enums are in byte order as they come in data buffer [5]...[8] */
typedef uint8_t avrshock2_axis_t;
#define AVRSHOCK2_AXIS_RX    ((avrshock2_axis_t)0x00)
#define AVRSHOCK2_AXIS_RY    ((avrshock2_axis_t)0x01)
#define AVRSHOCK2_AXIS_LX    ((avrshock2_axis_t)0x02)
#define AVRSHOCK2_AXIS_LY    ((avrshock2_axis_t)0x03)
#define AVRSHOCK2_AXIS_SIZE  (4)

/* avrshock2_analog_t are in byte order as they come in data buffer [9]...[20] */
typedef uint8_t avrshock2_analog_t;
#define AVRSHOCK2_ANALOG_RIGHT    ((avrshock2_analog_t)0x00)
#define AVRSHOCK2_ANALOG_LEFT     ((avrshock2_analog_t)0x01)
#define AVRSHOCK2_ANALOG_UP       ((avrshock2_analog_t)0x02)
#define AVRSHOCK2_ANALOG_DOWN     ((avrshock2_analog_t)0x03)
#define AVRSHOCK2_ANALOG_TRIANGLE ((avrshock2_analog_t)0x04)
#define AVRSHOCK2_ANALOG_CIRCLE   ((avrshock2_analog_t)0x05)
#define AVRSHOCK2_ANALOG_CROSS    ((avrshock2_analog_t)0x06)
#define AVRSHOCK2_ANALOG_SQUARE   ((avrshock2_analog_t)0x07)
#define AVRSHOCK2_ANALOG_L1       ((avrshock2_analog_t)0x08)
#define AVRSHOCK2_ANALOG_R1       ((avrshock2_analog_t)0x09)
#define AVRSHOCK2_ANALOG_L2       ((avrshock2_analog_t)0x0A)
#define AVRSHOCK2_ANALOG_R2       ((avrshock2_analog_t)0x0B)
#define AVRSHOCK2_ANALOGS_SIZE    (12)


#ifndef AVRSHOCK2_H_TYPES_ONLY /* AVRSHOCK2_H_TYPES_ONLY */
void avrshock2_init(void);
void avrshock2_set_mode(avrshock2_mode_t mode, bool lock);
bool avrshock2_poll(avrshock2_button_t* buttons,
                    avrshock2_axis_t axis[AVRSHOCK2_AXIS_SIZE]);


static inline avrshock2_mode_t avrshock2_get_mode(void)
{
	extern uint8_t avrshock2_data_buffer[33];
	return avrshock2_data_buffer[1];
}

#endif /* AVRSHOCK2_H_TYPES_ONLY */

#endif

