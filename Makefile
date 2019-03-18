CC=/usr/bin/avr-gcc
MEGA=328p
CFLAGS=-g -Os -Wall -mcall-prologues -mmcu=atmega$(MEGA)
OBJ2HEX=/usr/bin/avr-objcopy 
PROG=/usr/bin/avrdude
TARGET=binary
FILES=main.c

build: 
	# compile the source files
	$(CC) $(CFLAGS) $(FILES) -o $(TARGET).out -DF_CPU=16000000

	# convert the output into a hex file
	$(OBJ2HEX) -j .text -j .data -O ihex $(TARGET).out $(TARGET).hex

	# upload the hex file to the AVR
	$(PROG) -c usbtiny -p m$(MEGA) -U flash:w:$(TARGET).hex

