# Morse Beacon Makefile
MCU      = atmega328p
F_CPU    = 16000000UL
BAUD     = 115200

# Serial port to the Arduino bootloader.
#   Linux:   /dev/ttyUSB0  (CH340) or /dev/ttyACM0  (genuine Uno)
#   macOS:   /dev/cu.usbserial-XXXX  or  /dev/cu.usbmodemXXXX
#   Windows: COM3  (check Device Manager > Ports)
PORT     = /dev/ttyUSB0

CC       = avr-gcc
OBJCOPY  = avr-objcopy
CFLAGS   = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os  -Wall -Wextra -std=c11 
TARGET   = morse

all: $(TARGET).hex $(TARGET).bin


$(TARGET).elf: main.c
	$(CC) $(CFLAGS) -o $@ $<

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	avr-size --mcu=$(MCU) -C $<
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary -R .eeprom $< $@


flash: $(TARGET).hex
	avrdude -c arduino -p $(MCU) -P $(PORT) -b $(BAUD)  -U flash:w:$<

	

fuses:
	avrdude -c arduino -p $(MCU) -P $(PORT) -b $(BAUD) -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h

clean:
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).bin

.PHONY: all flash fuses clean
