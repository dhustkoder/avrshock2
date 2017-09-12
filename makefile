TARGET = ps2c
SRC = src/main.c src/uart.c
MCU = atmega328p
F_CPU = 16000000
#frequency for communication with the controller
F_PS2C = 500000
BAUD = 9600

# C
CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
CFLAGS = -std=gnu99 -Wall -Wextra -Wno-main -Os -flto -DNDEBUG -DPS2C_UART_DEBUG \
         -ffast-math -fstrict-aliasing -ffunction-sections -fdata-sections       \
         -fno-unwind-tables  -fno-asynchronous-unwind-tables -DNDEBUG            \
         -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL -DBAUD=$(BAUD) -DF_PS2C=$(F_PS2C)UL

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
