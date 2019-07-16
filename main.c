// main.c
// Written by Robert Leonard
// worm-v1.1 Hardware FCU

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define F_CPU 			16000000

#define TRIGGER_BIT		PORTB0
#define TRIGGER 		PINB

#define PROG_DDR_0      DDRC    /* DDR for the lower bits of the programming switches */
#define PROG_DDR_1      DDRD    /* DDR for the upper bits of the programming switches */
#define PROG_PORT_0     PORTC   /* Port for the lower bits of the programming switches */
#define PROG_PORT_1     PORTD   /* Port for the upper bits of the programming switches */
#define PROG_PIN_0      PINC    /* Pin for the lower bits of the programming switches */
#define PROG_PIN_1      PIND    /* Pin for the upper bits of the programming switches */
#define PROG_MSK_0      0xFF    /* TODO Bit mask for reading the lower bits of programming switches */
#define PROG_MSK_1      0xFF    /* TODO Bit mask for reading the upper bits of programming switches */

#define SOL_DDR			DDRB
#define SOL_BIT 		PORTB1

#define PRESCALER 		64		/* TIMER1 prescaler as defined by the datasheet */
#define DELAY_CONSTANT	10		/* Accounts for the response time of the solenoid */
#define DELAY_FACTOR	10		/* Changes the increment value of the prog. switches */

void pin_setup(void);
void interrupt_setup(void);
void power_setup(void);
void update_timer_counter(void);
uint16_t calc_compare_val(void);

// EFFECTS: Flag for determining if the system is handling a trigger interrupt sequence.
volatile uint8_t trigger_pulled_flag;

int main(void) {
	// Set up the power reduction registers
	power_setup();

	// Set up all pins for I/O
	pin_setup();
	interrupt_setup();

	// Get the timer counter value from the programming switches
	update_timer_counter();

	// Set the flag to false, indicating system is not handling a trigger pull.
	trigger_pulled_flag = 0;

	while(1) {
		if(!trigger_pulled_flag) {
			// Enable sleep and put the device under
			sleep_enable();
			sleep_mode();

			// Execution resumes here upon exit of waking interrupt
			sleep_disable();
		}
	}
}

// ISR:		Triggered by a change on the trigger pin.
// EFFECTS:	Solenoid is energized on trigger rising edge
// NOTE:	OC1A is preset to set on output compare
ISR(PCINT0_vect) {
	// Do nothing if the trigger has been released
	if(!(TRIGGER & _BV(TRIGGER_BIT))) return;

	// Do nothing if currently handling trigger pull
	if(trigger_pulled_flag) return;

	// System is now handling the trigger sequence
	trigger_pulled_flag = 1;

	// Force output compare to set solenoid
	TCCR1C |= _BV(FOC1A);

	// Enable clear OC1A on output compare
	TCCR1A &= ~_BV(COM1A0);

	// Enable TIMER1, pre-scalar=64
	TCCR1B |= _BV(CS11) | _BV(CS10);
}

// ISR:		Triggered by a match compare on TIMER1 (CTC mode non-PWM)
// EFFECTS:	Enables set OC1A on match and disables and resets TIMER1. Updates timer counter from switches.
// NOTE:	OC1A clears in hardware immediately on match
ISR(TIMER1_COMPA_vect) {
	// Disable and reset TIMER1
	TCCR1B &=  ~_BV(CS11) & ~_BV(CS10);
	TCNT1 = 0;

	// Enable set OC1A on match
	TCCR1A |= _BV(COM1A0);

	// Update the counter value from the programming switches
	update_timer_counter();

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
	// TRIGGER INTERRUPT (Input, Pull-up Disabled)
	// No operations here, DDR and PORT initialize to correct values by default

	// PROGRAMMING SWITCHES [7:0] (Input, Pull-up Enabled; DDR defaults to 0)
	PROG_PORT = 0xFF;

	// SOLENOID (Output, preset OC1A to set on match)
	SOL_DDR |= _BV(SOL_BIT);
	TCCR1A |= _BV(COM1A1) | _BV(COM1A0);

	// UNUSED PINS (Input, Pullup Enabled)
	// TODO: all remaining unused pins should be pulled up inputs
	// ...
}

// EFFECTS: Initializes the interrupt registers for trigger, timer, and prog. switches
void interrupt_setup(void) {
	// TRIGGER EXTERNAL INTERRUPT
	PCICR |= _BV(PCIE0);		// Enable the PCI-0 ISR (PORTB)
	PCMSK0 |= _BV(TRIGGER_BIT);	// Enable PORTB0 for PCINT (PCINT0)

	// TRIGGER TIMER INTERRUPT
	TCCR1B |= _BV(WGM12);		// Setup timer for CTC mode
	OCR1A = calc_compare_val(); // Initialize the compare value
	TIMSK1 |= _BV(OCIE1A);		// Enable the CTC interrupt vector

	// PROGRAMMING SWITCHES INTERRUPT
	PCICR |= _BV(PCIE2);		// Enable the PCI-2 ISR (PORTD)
	PCMSK2 = 0xFF;				// Enable all pins on PCI[23:16] (PORTD) as interrupts

	// SETUP
	sei();						// Enable global interrupts
}

// EFFECTS: Modifies necessary registers for reducing power consumption
void power_setup(void) {
	// Disable the I2C interface
	PRR |= _BV(PRTWI);

	// Disable timer 0 and 2
	PRR |= _BV(PRTIM0);
	PRR |= _BV(PRTIM2);

	// Disable the SPI module
	PRR |= _BV(PRSPI);

	// Disable the USART module
	PRR |= _BV(PRUSART0);

	// Disable the ADC
	PRR |= _BV(PRADC);

	// Disable the WDT
	MCUSR &= ~_BV(WDRF);
	WDTCSR &= ~_BV(WDIE);
	WDTCSR &= ~_BV(WDE);

	// Set the sleep mode to power down
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

// EFFECTS: Enables, reads, and disables the programming switches to update the counter value
void update_timer_counter(void) {
	// Enable the pull-up resistors for the programming switches
	// TODO

	// Read the programming switches
	// TODO

	// Calculate the new compare value
	// TODO

	// Disable the pull-up resistors on the programming switches
	// TODO
}

// EFFECTS: Computes the required output compare value for the CTC timer
//			given the desired millisecond input on the PROG switches.
// NOTE:	Programming switches are a binary representation of 0.1ms increments.
uint16_t calc_compare_val(void) {
	return (uint16_t)(F_CPU / (1000 * DELAY_FACTOR) / PRESCALER *
					  ((uint8_t)~PROG + DELAY_CONSTANT));
}
