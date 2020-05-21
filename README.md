## AVRSHOCK2
AVRSHOCK2 is a small API to use the PS2 Controller through AVR Microcontrollers.
To use the AVRSHOCK2, just add the avrshock2.c and avrshock2.h files to your project.

### Example
The makefile builds an example of how to use the AVRSHOCK2 API by logging to serial port the pressed buttons, it was tested on Atmega328p (Arduino UNO board).

### [Example Project](https://github.com/dhustkoder/avrshock2-usb)
[![gif](https://user-images.githubusercontent.com/11935784/36216949-55bdb2fc-1197-11e8-9f23-88adfcffa6d4.gif)](https://github.com/dhustkoder/avrshock2-usb)



### Details
[![gif](https://user-images.githubusercontent.com/11935784/30407023-2046349e-98cc-11e7-970d-6c117b176b94.gif)](https://www.youtube.com/watch?v=_h1dANNXZOw)

AVRSHOCK2 uses 4 pins to communicate with the controller: ATT, CMD, DAT, CLK.
If you use SPI Mode: SS = ATT, MOSI = CMD, MISO = DAT, SCK = CLK.
You should set the PORT, DDR, BIT for them by defining: AVRSHOCK2_PORT_ATT, AVRCHOCK2_DDR_ATT, AVRSHOCK2_BIT_ATT etc... For DAT you should set the PIN also: AVRSHOCK2_PIN_DAT (see makefile).
F_AVRSHOCK2 is the rate in hertz that the communication will be made, range (100000 - 500000).
If not defined F_AVRSHOCK2 defaults to 250000.

Here is the circuit diagram to use AVRSHOCK2:

![wiring](https://user-images.githubusercontent.com/11935784/30391091-a2fdcd18-988e-11e7-9561-003a0468e4dc.jpg "Dualshock 2 wires")
![diagram](https://user-images.githubusercontent.com/11935784/30390726-6c283162-988d-11e7-8999-d177818f56dc.png "AVR to Dualshock Diagram")


