TARGET = ps2c
SRC = src/main.c src/ps2c.c src/uart.c
MCU = atmega328p
F_CPU = 16000000
BAUD = 9600

# C
CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
CFLAGS = -std=c11 -pedantic-errors -Wall -Wextra -Wno-main -DNDEBUG -DPS2C_UART_DEBUG  \
	 -Os -flto -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums    \
         -ffast-math -fstrict-aliasing -ffunction-sections -fdata-sections             \
	 -fwhole-program -ffreestanding -fno-tree-scev-cprop                           \
	 -Wl,--relax -fno-unwind-tables  -fno-asynchronous-unwind-tables               \
         -DNDEBUG -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL -DBAUD=$(BAUD)

# AVRDUDE
AVRDUDE = avrdude
AVRDUDE_PORT = /dev/ttyUSB0
AVRDUDE_PROGRAMMER = arduino


all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LIBS) $(SRC) -o $(TARGET).elf
	$(OBJCOPY) -j .text  -j .data -O ihex $(TARGET).elf $(TARGET).hex
	$(SIZE) $(TARGET).elf


program: $(TARGET)
	$(AVRDUDE) -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -b 115200 -U flash:w:$(TARGET).hex:i

clean:
	rm -rf ps2c.*
