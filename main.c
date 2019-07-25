// main.c
// Written by Robert Leonard
// worm-v1.1 Hardware FCU

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define F_CPU 			12000000

#define TRIGGER_BIT		PORTB0
#define TRIGGER 		PINB

#define TRIGGER_PU_PORT PORTB   /* PORT for the pre-inverter trigger internal pull-up */
#define TRIGGER_PU_BIT  PORTB1  /* Bit number for the pull-up */

#define PROG_PORT_0     PORTC   /* Port for the lower bits of the programming switches */
#define PROG_PORT_1     PORTD   /* Port for the upper bits of the programming switches */
#define PROG_PIN_0      PINC    /* Pin for the lower bits of the programming switches */
#define PROG_PIN_1      PIND    /* Pin for the upper bits of the programming switches */
#define PROG_MSK_0      0x7F    /* Bit mask for reading the lower bits of programming switches */
#define PROG_MSK_1      0x07    /* Bit mask for reading the upper bits of programming switches */

#define C_UNUSED_MSK    0x00    /* Bit mask for the unused pins on PORTC */
#define D_UNUSED_MSK    0x1F    /* Bit mask for the unused pins on PORTD */

#define VALVE_DDR		DDRB    /* Solenoid valve port */
#define VALVE_BIT 		PORTB1  /* Solenoid valve bit */

// TODO
#define CLK_SCALER      ( (1 << CS11) | (1 << CS10) )
#define CLKSEL          0xFF    /* TIMER1 prescaler mask */
#define PRESCALER 		64		/* TIMER1 prescaler as defined by the datasheet */
#define DELAY_CONSTANT	10		/* Accounts for the response time of the solenoid */
#define DELAY_FACTOR	10		/* Changes the increment value of the prog. switches */

void pin_setup(void);
void interrupt_setup(void);
void power_setup(void);
void update_timer_counter(void);
void enable_programming_switches(void);
uint16_t calc_compare_val(uint8_t prog_setting);

// EFFECTS: Flag for determining if the system is handling a trigger interrupt sequence.
volatile uint8_t trigger_pulled_flag;

int main(void) {
	// Set up the power reduction registers
	power_setup();

	// Set up all I/O pins
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
// NOTE:	OC1B is preset to set on output compare
ISR(PCINT0_vect) {
	// Do nothing on a trigger rising edge
	if(!(TRIGGER & _BV(TRIGGER_BIT))) return;

	// Do nothing if currently handling trigger pull
	if(trigger_pulled_flag) return;

	// System is now handling the trigger sequence
	trigger_pulled_flag = 1;

	// Force output compare to set solenoid
	TCCR1C |= _BV(FOC1B);

	// Enable clear OC1A on output compare
	TCCR1A &= ~_BV(COM1B0);

	// Enable TIMER1, pre-scalar=64
	TCCR1B |= CLK_SCALER;
}

// ISR:		Triggered by a match compare on TIMER1 (CTC mode non-PWM)
// EFFECTS:	Enables set OC1B on match and disables and resets TIMER1. Updates timer counter from switches.
// NOTE:	OC1B clears in hardware immediately on match
ISR(TIMER1_COMPA_vect) {
	// Disable and reset TIMER1
	TCCR1B &=  ~CLK_SCALER;
	TCNT1 = 0;

	// Enable set OC1B on match
	TCCR1A |= _BV(COM1B0);

	// Update the counter value from the programming switches
	update_timer_counter();

	// System has completed handling the trigger sequence
	trigger_pulled_flag = 0;
}

// EFFECTS: Sets the data direction and pull-ups for each IO pin
void pin_setup(void) {
	// TRIGGER INTERRUPT (Input, Pull-up Disabled)
	// No operations here, DDR and PORT initialize to correct values by default

	// TRIGGER SWITCH (Input, Pull-up Enabled)
	// Note: This value is never read, but the internal pull-up is used
	TRIGGER_PU_PORT |= _BV(TRIGGER_PU_BIT);

	// PROGRAMMING SWITCHES [7:0] (Input, Pull-up Disabled; DDR defaults to 0)
	// No operations here, DDR and PORT initialize to correct values by default

	// SOLENOID (Output, attach to OC1B and preset to set on timer compare match)
	VALVE_DDR |= _BV(VALVE_BIT);
	TCCR1A |= _BV(COM1B1) | _BV(COM1B0);

	// UNUSED PINS (Input, Pullup Enabled)
	// TODO: all remaining unused pins should be pulled up inputs
	// ...
}

// EFFECTS: Initializes the interrupt registers for the trigger and timer
void interrupt_setup(void) {
	// TRIGGER EXTERNAL INTERRUPT
	// TODO: port to hv1.1
	PCICR |= _BV(PCIE0);		// Enable the PCI-0 ISR (PORTB)
	PCMSK0 |= _BV(TRIGGER_BIT);	// Enable PORTB0 for PCINT (PCINT0)

	// TRIGGER TIMER INTERRUPT
	// TODO: port to hv1.1
	TCCR1B |= _BV(WGM12);		// Setup timer for CTC mode
	OCR1A = calc_compare_val(0); // Initialize the compare value
	TIMSK1 |= _BV(OCIE1A);		// Enable the CTC interrupt vector

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

	// Disable the pull-up resistors on the programming switches
	// TODO

	// Calculate and set the new compare value
	// TODO
}

// EFFECTS: enables the internal pull-ups on the programming switches
void enable_programming_switches(void) {
	PROG_PORT_0 |= PROG_MSK_0;
	PROG_PORT_1 |= PROG_MSK_1;
}

// EFFECTS: Computes the required output compare value for the CTC timer
//			given the desired millisecond input on the PROG switches.
// NOTE:	Programming switches are a binary representation of 0.1ms increments.
uint16_t calc_compare_val(uint8_t prog_setting) {
	return (uint16_t)(F_CPU / (1000 * DELAY_FACTOR) / PRESCALER *
					  (prog_setting + DELAY_CONSTANT));
}
