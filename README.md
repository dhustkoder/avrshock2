## PS2C
PS2C is a small API to use the PS2 Controller through AVR Microcontrollers.
To use the PS2C, just add the ps2c.c and ps2c.h source files to your project.

### Example
The makefile builds an example of how to use the PS2C API by manipulating LEDs using the analog stick, it was tested on Atmega328p (Arduino UNO board).
Look in main.c for pins used to light LEDs, and ps2c.c for the pins that communicate with the controller.

[![giff](https://user-images.githubusercontent.com/11935784/30407023-2046349e-98cc-11e7-970d-6c117b176b94.gif)](https://www.youtube.com/watch?v=_h1dANNXZOw)

### Details
PS2C uses 4 pins to communicate with the controller, select these pins by editing
the definitions in ps2c.c source file.
F_PS2C is the rate in hertz that the communication will be made, range (100000 - 500000).
F_PS2C will default to 250000 if not defined. edit the F_PS2C definition in ps2c.c, or define it in your compiler flags.

Here is the circuit diagram to use PS2C:

![wiring](https://user-images.githubusercontent.com/11935784/30391091-a2fdcd18-988e-11e7-9561-003a0468e4dc.jpg "Dualshock 2 wires")
![diagram](https://user-images.githubusercontent.com/11935784/30390726-6c283162-988d-11e7-8999-d177818f56dc.png "AVR to Dualshock Diagram")


