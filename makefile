EXAMPLE = avrshock2-example
EXAMPLE_SOURCES = src/example.c src/uart.c
EXAMPLE_HEADERS = src/uart.h
LIB_SOURCES = src/avrshock2.c
LIB_HEADERS = src/avrshock2.h

MCU = atmega328p
F_CPU = 16000000
BAUD = 9600

# C
CC = avr-gcc
AR = avr-ar
OBJCOPY = avr-objcopy
SIZE = avr-size
CFLAGS = -std=c11 -pedantic-errors -Wall -Wextra -Wno-main -DNDEBUG -DAVRSHOCK2_UART_DEBUG  \
	 -Os -flto -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums         \
         -ffast-math -fstrict-aliasing -ffunction-sections -fdata-sections                  \
	 -fwhole-program -ffreestanding -fno-tree-scev-cprop                                \
	 -Wl,--relax -fno-unwind-tables  -fno-asynchronous-unwind-tables                    \
         -DNDEBUG -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL -DBAUD=$(BAUD)

# AVRDUDE
AVRDUDE = avrdude
AVRDUDE_PORT = /dev/ttyUSB0
AVRDUDE_PROGRAMMER = arduino

all: $(EXAMPLE)

$(EXAMPLE): $(LIBAVRSHOCK2) $(EXAMPLE_SOURCES) $(EXAMPLE_HEADERS) $(LIB_SOURCES) $(LIB_HEADERS)
	$(CC) $(CFLAGS) $(EXAMPLE_SOURCES) $(LIB_SOURCES) -o $(EXAMPLE).elf
	$(OBJCOPY) -j .text  -j .data -O ihex $(EXAMPLE).elf $(EXAMPLE).hex
	$(SIZE) $(EXAMPLE).elf


program: $(EXAMPLE)
	$(AVRDUDE) -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -b 115200 -U flash:w:$(EXAMPLE).hex:i


clean:
	rm -rf *.elf *.hex
