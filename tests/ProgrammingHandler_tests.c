//
// ProgrammingHandler_tests.c
// Created by Robert Leonard on 2/3/19.
//

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "../ProgrammingHandler.h"

// timer setting set by the DIP switch ISR and used by the trigger ISR
volatile unsigned char timer_setting;

int main(void) {
	// Set all of PORTD to inputs with pullup resistors enabled
	// These are the programming switches [7:0]
	DDRD = 0x0;
	PORTD = 0xFF;

	// Set all of PORTB to outputs
	DDRB = 0xFF;			// all of PORTB are outputs
	PORTB &= ~_BV(PORTB0);	// set PB0 to LOW

	PCICR |= _BV(PCIE2);
	PCMSK2 = 0xFF;
	// Turn interrupts on.
	sei();

	timer_setting = read_timer_switches();


	while(1) {


		for(unsigned char i = 0; i < timer_setting; i++) {
			PORTB |= _BV(PORTB0);
			_delay_ms(1);
			PORTB &= ~_BV(PORTB0);
			_delay_ms(1);
		}

		_delay_ms(100);
	}
}

ISR(PCINT2_vect) {
	timer_setting = read_timer_switches();
}
