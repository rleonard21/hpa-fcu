//
// ProgrammingHandler.c
// Created by Robert Leonard on 2/3/19.
//

#include "ProgrammingHandler.h"
#include <avr/io.h>

#define SW_PORT PORTD

// EFFECTS: reads the port of the timer switches and converts to decimal value
unsigned char read_timer_switches(void) {
	return (unsigned char)~PIND;
}
