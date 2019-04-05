// main.c
// Written by Robert Leonard

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 			16000000

#define TRIGGER_DDR		DDRB
#define TRIGGER_PORT	PORTB
#define TRIGGER_BIT		PORTB0
#define TRIGGER 		PINB

#define PROG_DDR		DDRD
#define PROG_PORT		PORTD
#define PROG			PIND

#define SOL_DDR			DDRB
#define SOL_PORT		PORTB
#define SOL_BIT 		PORTB1

#define PRESCALER 		64		/* TIMER1 prescaler as defined by the datasheet */
#define DELAY_CONSTANT	100		/* Accounts for the response time of the solenoid */
#define DELAY_FACTOR	10		/* Changes the increment value of the prog. switches */

void pin_setup(void);
void interrupt_setup(void);
void power_setup(void);
uint16_t calc_compare_val(void);

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

// ISR:		Triggered by a change on the trigger pin.
// EFFECTS:	Solenoid is energized on trigger rising edge
ISR(PCINT0_vect) {
	if(!(TRIGGER & _BV(TRIGGER_BIT))) return;	// Do nothing if the trigger has been released
	if(trigger_pulled_flag) return;		// Do nothing if currently handling trigger pull

	trigger_pulled_flag = 1;			// System is now handling the trigger sequence

	// TODO: implement the following comments to (energize the solenoid by force compare)
	// Force output compare (timer control register should be preset to set OC1A on match prior to this ISR)
	// Enable reset OC1A on output compare	

	TCCR1B |= (1 << CS11)|(1 << CS10);	// Enable TIMER1, pre-scalar=64
}

// ISR:		Triggered by a match compare on TIMER1 (CTC mode non-PWM)
// EFFECTS:	Enables set OC1A on match and disables and resets TIMER1.
ISR(TIMER1_COMPA_vect) {
	// Disable and reset TIMER1
	TCCR1B &=  ~_BV(CS11) & ~_BV(CS10);
	TCNT1 = 0;

	// TODO: implement the following comments:
	// Enable set OC1A on match (OC1A is force matched by trigger)

	// System has completed handling the trigger sequence
	trigger_pulled_flag = 0;
}

// ISR:	    Triggered by any change in the programming switches (PCINT2)
// EFFECTS: Sets the CTC compare value to the value calculated from the switches
ISR(PCINT2_vect) {
	OCR1A = calc_compare_val();
}

// EFFECTS: Sets the data direction and pull-ups for each IO pin
void pin_setup(void) {
	// TRIGGER INTERRUPT (Input, Pullup Disabled)
	TRIGGER_DDR &= ~_BV(TRIGGER_BIT);
	TRIGGER_PORT &= ~_BV(TRIGGER_BIT);

	// PROGRAMMING SWITCHES [7:0] (Input, Pullup Enabled)
	PROG_DDR = 0x0;
	PROG_PORT = 0xFF;
	
	// TODO: probably delete this because solenoid is mux'd to OC1A output
	// SOLENOID (Output, initially LOW)
	SOL_DDR |= _BV(SOL_BIT);
	SOL_PORT &= ~_BV(SOL_BIT);

	// UNUSED PINS (Input, Pullup Enabled)
	// TODO: all remaining unused pins should be pulled up inputs
	// ...
}

// EFFECTS: Initializes the interrupt registers for trigger, timer, and prog. switches
void interrupt_setup(void) {
	// TRIGGER EXTERNAL INTERRUPT
	PCICR |= _BV(PCIE0);		// Enable the PCI-0 ISR (PORTB)
	PCMSK0 |= _BV(TRIGGER_BIT);	// Enable PORTB0 for PCINT (PCINT0)

	// TODO: enable set OC1A on match
	// TRIGGER TIMER INTERRUPT
	TCCR1B |= (1 << WGM12);		// Setup timer for CTC mode
	TCNT1 = 0;					// Initialize the counter
	OCR1A = calc_compare_val(); // Initialize the compare value
	TIMSK1 |= (1 << OCIE1A);	// Enable the CTC interrupt vector

	// PROGRAMMING SWITCHES INTERRUPT
	PCICR |= _BV(PCIE2);		// Enable the PCI-2 ISR (PORTD)
	PCMSK2 = 0xFF;				// Enable all pins on PCI[23:16] (PORTD) as interrupts

	// SETUP
	sei();						// Enable global interrupts
}

// EFFECTS: Modifies necessary registers for reducing power consumption
void power_setup(void) {
	// TODO: implement this function
}

// EFFECTS: Computes the required output compare value for the CTC timer
//			given the desired millisecond input on the PROG switches.
// NOTE:	Programming switches are a binary representation of 0.1ms increments.
uint16_t calc_compare_val(void) {
	return (uint16_t)(F_CPU / (1000 * DELAY_FACTOR) / PRESCALER *
					  ((uint8_t)~PROG + DELAY_CONSTANT));
}
