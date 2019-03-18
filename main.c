// main.c
// Written by Robert Leonard

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 		16000000

#define PROG		PINB
#define PROG_PORT	PORTB
#define PROG_DDR	DDRB

#define SOL_PORT	PORTD
#define SOL_BIT 	PORTD0

#define PRESCALER 	64

void pin_setup(void);
void interrupt_setup(void);
uint16_t calc_compare_val();

// Flag for determining if the system is handling a trigger interrupt sequence.
volatile uint8_t trigger_pulled_flag;

int main(void) {
	// Set up all pins for I/O
	pin_setup();
	interrupt_setup();

	// Set the flag to false, indicating system is not handling a trigger pull.
	trigger_pulled_flag = 0;

	while(1) {
		if(!trigger_pulled_flag) {
			// System is not handling a trigger pull, so put device to sleep
			// TODO: put device into power-save mode
		}
	}
}

// ISR:     triggered by a trigger pull (low level on INT0)
// EFFECTS: energizes the solenoid and starts the 16-bit CTC mode timer
ISR(INT0_vect) {
	trigger_pulled_flag = 1;	// System is now handling the trigger sequence

	SOL_PORT |= _BV(SOL_BIT);			// Energize the solenoid
	TCCR1B |= (1 << CS11)|(1 << CS10);	// Enable TIMER1, pre-scalar=64
}

// ISR:		triggered by a match compare on TIMER1 (CTC mode non-PWM)
// EFFECTS:	de-energizes solenoid, disables and resets TIMER1.
ISR(TIMER1_COMPA_vect) {
	SOL_PORT &= ~_BV(SOL_BIT);	// de-energize the solenoid

	TCCR1B &=  ~_BV(CS11) & ~_BV(CS10) & ~_BV(CS00);	// disable TIMER1
	TCNT1 = 0;	// Reset TIMER1 to zero

	trigger_pulled_flag = 0;	// System has handled the trigger sequence
}

// ISR:	    triggered by any change in the programming switches (PCINT)
// EFFECTS: sets the CTC compare value to the calculated timer value
ISR(PCINT2_vect) {
	OCR1A = calc_compare_val();
}

// EFFECTS: sets the data direction and pull-ups for each IO pin
void pin_setup(void) {
	// TRIGGER INTERRUPT (Input)
	// TODO: trigger interrupt pin setup
	// ...

	// PROGRAMMING SWITCHES [7:0] (Input, Pullup Enabled)
	PROG_DDR = 0x0;
	PROG_PORT = 0xFF;

	// UNUSED PINS (Input, Pullup Enabled)
	// TODO: all remaining unused pins should be pulled up inputs
	// ...
}

// EFFECTS: initializes the interrupt registers for trigger, timer, and prog. switches
void interrupt_setup(void) {
	// TRIGGER EXTERNAL INTERRUPT
	EICRA = 0x00;			// ISR triggers on low level
	EIMSK |= (1 << INT0);   // Enable the INT0 interrupt vector

	// TRIGGER TIMER INTERRUPT
	TCCR1B |= (1 << WGM12);		// Setup timer for CTC mode
	TCNT1 = 0;					// Initialize the counter
	OCR1A = calc_compare_val(); // Initialize the compare value
	TIMSK1 |= (1 << OCIE1A);	// Enable the CTC interrupt vector

	// PROGRAMMING SWITCHES INTERRUPT
	PCICR |= _BV(PCIE0);	// Enable the PCI-0 ISR (PORTB)
	PCMSK0 = 0xFF;			// Enable all pins on PCI[7:0] (PORTB) as interrupts

	// SETUP
	sei();	// Enable global interrupts
}

// EFFECTS: Computes the required output compare value for the CTC timer
//			given the desired millisecond input on the PROG switches.
uint16_t calc_compare_val() {
	return (uint16_t)((F_CPU * ~PROG) / (1000 * PRESCALER));
}
