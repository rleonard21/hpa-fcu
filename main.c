// main.c
// Written by Robert Leonard

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void pin_setup(void);
void interrupt_setup(void);

// Timer setting set by the DIP switch ISR and used by the trigger ISR
volatile uint8_t timer_setting;

// Flag for determining if an ISR was triggered.
volatile unsigned char isr_flag;

int main(void) {
	// Set up all pins for I/O
	pin_setup();
	interrupt_setup();

	// Set the timer to the value of the programming switches
	timer_setting = (uint8_t)~PINB;

	// Set the ISR flag to false, indicating no ISR was handled.
	isr_flag = 0;

	// TODO: start timer

	// Do nothing, only handle interrupts as they come in
	while(1);
}

// ISR:     triggered by a trigger pull (low level on INT0)
// EFFECTS: energizes the solenoid and starts the 16-bit CTC mode timer
ISR(INT0_vect) {
	PORTB = 0xFF;				// Energize the solenoid
	// TODO: turn on the specific INT0 bit on PORTB, not all of PORTB (don't know which pin yet)
	// TODO: start 16-bit counter and exit
}

// TODO: 16 bit counter ISR for trigger
// TODO: turn off OCxy pin when interrupt happens (can be done in hardware)
// TODO: disable the 16 bit counter (and reset the counter?)

// ISR:	    triggered by any change in the programming switches (PCINT on B register)
// EFFECTS: converts the 8-bit B register to an unsigned char for the timer setting
ISR(PCINT2_vect) {
	timer_setting = (uint8_t)~PINB;
}

// EFFECTS: sets the data direction and pullups for each IO pin
void pin_setup(void) {
	// TRIGGER INTERRUPT (Input)
	// TODO: trigger interrupt pin setup
	// ...
	
	// PROGRAMMING SWITCHES [7:0] (Input, Pullup Enabled)
	DDRB = 0x0;
	PORTB = 0xFF;

	// UNUSED PINS (Input, Pullup Enabled)
	// TODO: all remaining unused pins should be pulled up inputs
	// ...
}

// EFFECTS: intializes the interrupt registers for INT0 and PCINT2 (trigger and switches)
void interrupt_setup(void) {
	// TRIGGER INTERRUPT
	EICRA = 0x00;			// ISR triggers on low level
	EIMSK |= (1 << INT0);   // Enable INT0

	// PROGRAMMING SWITCHES INTERRUPT
	PCICR |= _BV(PCIE0);	// Enable the PCI-0 ISR (PORTB)
	PCMSK0 = 0xFF;			// Enable all pins on PCI[7:0] (PORTB) as interrupts

	// SETUP
	sei();	// Enable global interrupts
}
