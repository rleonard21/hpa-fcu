# HPA Fire Control Unit (OC1A Branch)

## What is this Branch?
This branch is the implementation of the FCU using the `OC1A` pin functionality
to turn on and off the solenoid valve. The benefit of this implementation is
that the solenoid is turned off synchronously with the timer match with 
hardware, which should (in theory) beat the master branch's interrupt 
driven system that toggles the solenoid by moving bits into registers. 
In practice, this theory holds. The `OC1A` hardware variation of the firmware
is indeed around 1us faster than the firmware driven toggle. However, this
difference between the two implementations is insignificant compared to
the slew rate of the solenoid. The solenoid will take orders of magnitude
more time to move than the timing difference between the two versions of the
firmware. I have chosen to continue without the hardware-based `OC1A` toggling
because the firmware is larger by around ~30 bytes and because the cheaper
ATMega328PB does not support the necessary force output compare functionality
present in the ATMega328/P. The benefits of the `OC1A` implementation are
somewhat insignificant and the downsides are non-negligible.

## Hardware Description
The firmware runs on an ATMega328P-AU AVR microcontroller (the firmware
is under 500 bytes when compiled with `-Os`, so a smaller MCU may be
warranted). The ATMega328PB will not work with this firmware due to a lack
of the force output compare function for the 16-bit timer.
The trigger is a snap-action limit switch that is attached
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
precise pulse for the solenoid valve. The solenoid is activated by the
in the trigger interrupt and deactivated by hardware in when the timer
matches the specified value.

#### Trigger Pull
The rising edge from a trigger pull fires a pin change
interrupt on `PORTB0`. Falling edges are ignored. In the
corresponding interrupt service routine (ISR), the system drives the
solenoid and starts the 16-bit `TIMER1`. The timer is set up for
CTC mode, non-PWM functionality during power-on. The computation
for the timer compare value is outlined in the "Programming Switches
and Delay" section. After the timer is started, the trigger pull ISR
exits and the system runs idle. When the timer reaches the compare
value, the solenoid is deactivated in hardware on the `OC1A` pin and
a timer match interrupt is fired. This interrupt turns stops and resets 
the timer. The timer interrupt also presents the `OC1A` pin to set when
the next compare happens, which will be when the trigger is pulled. 
The system
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
