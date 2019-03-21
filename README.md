# HPA Fire Control Unit
## Hardware Description
The firmware runs on an ATMega328P-AU AVR microcontroller (the firmware
is under 500 bytes when compiled with `-Os`, so a smaller MCU may be
warranted). The trigger is a snap-action limit switch that is attached
directly to the rifle's trigger housing via a PCB. The trigger is debounced
entirely in hardware, which enables for direct interrupt control on the MCU
and simpler code overall. The trigger debouncing consists of an RC-filter
pass through a Schmitt trigger to produce an excellent digital signal for
the MCU. THe programming switches are not debounced and do not have to be
because having the ISR for the switches run more than once does not affect
system's functionality. Not having any debounce on the switches reduces the
size and cost of the PCB and the complexity of the firmware. 

## How it Works
The system maximizes its use of the AVR's hardware features to
squeeze the best performance out of the chip. These features include
a true real-time system that is entirely interrupt driven. In
addition, a hardware timer is used to create an accurate and
precise pulse for the solenoid valve. 

#### Trigger Pull
The rising edge from a trigger pull fires a pin change
interrupt on `PORTB0`. Falling edges are ignored. In the
corresponding interrupt service routine (ISR), the system drives the
solenoid and starts the 16-bit `TIMER1`. The timer is set up for
CTC mode, non-PWM functionality during power-on. The computation
for the timer compare value is outlined in the "Programming Switches
and Delay" section. After the timer is started, the trigger pull ISR
exits and the system runs idle. When the timer reaches the compare
value, a timer match interrupt is fired. This interrupt turns off
the solenoid and then stops and resets the timer. The system
subsequently waits for the next interrupt to occur. During the period
between the handling of the trigger ISR and the timer ISR, a flag
is set to indicate that the firing sequence is being handled. This
flag prevents the system from handling any additional trigger pulls
while the firing sequence is happening, which prevents problems such
as a mistimed solenoid release.    

#### Programming Switches and Delay
The programming
switches control the value stored in the `PORTD` register, which
represents the desired delay time. A change in any of the
programming switches fires a pin change interrupt, whose
corresponding ISR calculates the new timer compare value for the
new switch settings. A delay constant is added to the time
represented by the switches to account for the lag of the solenoid
valve.
