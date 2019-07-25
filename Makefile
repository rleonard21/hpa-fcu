CC=/usr/local/bin/avr-gcc
MEGA=328p
CFLAGS=-g -Os -Wall -mcall-prologues -mmcu=atmega$(MEGA)
OBJ2HEX=/usr/local/bin/avr-objcopy 
PROG=/usr/local/bin/avrdude
TARGET=binary
FILES=main.c hotwire/Hotwire.c lcd/lcd.c lcd/LCDControl.c encoder/Debounce.c encoder/Encoder.c feedback/Buzzer.c \
lcd/ViewController.c Interface.c lcd/StringUtility.c

.DEFAULT_GOAL = build

build: 
	# compile the source files
	$(CC) $(CFLAGS) $(FILES) -o $(TARGET).out -DF_CPU=16000000

	# convert the output into a hex file
	$(OBJ2HEX) -j .text -j .data -O ihex $(TARGET).out $(TARGET).hex

	# upload the hex file to the AVR
	$(PROG) -c usbtiny -p m$(MEGA) -U flash:w:$(TARGET).hex

fuse:
	# sets the fuses for full swing oscillator, no clock divide
	$(PROG) -c usbtiny -p m$(MEGA) -U lfuse:w:0xe7:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m
