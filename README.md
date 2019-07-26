# HPA Fire Control Unit

## Hardware Description
The firmware runs on an ATMega48/88/168/328/P/PB AVR microcontroller (the firmware
is around 700 bytes when compiled with `-Os`, so a smaller MCU may be
warranted). The trigger is a snap-action limit switch that is attached
directly to the rifle's trigger housing via a PCB. The trigger is debounced
entirely in hardware, which enables for direct interrupt control on the MCU
and simpler code overall. The hardware debouncing also enables the MCU to enter full
power-down mode, which saves a significant amount of current draw.
The trigger debouncing consists of an RC-filter
passed through a Schmitt trigger to produce an excellent digital signal for
the MCU's interrupt. The programming switches are not debounced and do not have to be
because they are polled only after a trigger pull. It would be quite difficult to enter a 
state where the toggle switches are bouncing directly after a trigger pull.
Not having any debounce on the switches reduces the
size and cost of the PCB (if debounced in hardware) or the complexity of 
the firmware (if debounced in software). 

## How it Works
The system maximizes its use of the AVR's hardware features to
squeeze the best performance out of the chip. These features include
a true real-time system that is entirely interrupt driven. In
addition, a hardware timer is used to create an accurate and
precise pulse for the solenoid valve. The solenoid is activated by the trigger interrupt 
and deactivated in hardware when the timer matches the specified compare value.

#### Trigger Pull
The rising edge from a trigger pull fires a pin change
interrupt on `PORTB0`. Falling edges are ignored. In the
corresponding interrupt service routine (ISR), the system drives the
solenoid and starts the 16-bit `TIMER1`. The timer is set up for
CTC mode, non-PWM functionality during power-on. The computation
for the timer compare value is outlined in the "Programming Switches
and Delay" section. After the timer is started, the trigger pull ISR
exits and the system runs idle. When the timer reaches the compare
value, the solenoid is deactivated in hardware on the `OC1B` pin and
a timer match interrupt is fired. This interrupt turns stops and resets 
the timer. The timer interrupt also presents the `OC1B` pin to set when
the next compare happens, which will be when the trigger is pulled. 
The system
subsequently powers down and waits for the next interrupt to occur. During the period
between the handling of the trigger ISR and the timer ISR, a flag
is set to indicate that the firing sequence is being handled. This
flag prevents the system from handling any additional trigger pulls
while the firing sequence is happening.

#### Programming Switches and Delay
The programming
switches control the desired delay time. The state of the switches is polled after every trigger release.
Therefore, any new settings will not take effect until the trigger is pulled. A delay constant is added to the time
represented by the switches to account for the lag of the solenoid valve.

### worm v1.1
This hardware revision of the FCU improves upon a few aspects of `v1.0` hardware. The battery measurement circuit and supporting LEDs have been removed to reduce BOM cost, soldering time, and overall complexity. The programming switches are now half pitch and placed on the top side of the PCB, such that the entire circuit can be soldered with hot air. Only the flying JST power leads require hand soldering. Additionally, the programming pins now use Pogo pins instead of a soldered header. 

![worm v1.1](https://imgur.com/EAgREVe.jpg)

### worm v1.0
This unit is the first revision of the fire control unit. It includes the features described above and additional circuitry for detecting low battery. The low battery detection does not function (I used an N-channel MOSFET as a high side switch...). The `worm v1.0` hardware draws 3.4uA when powered with 5v. 

![worm v1.0](https://i.imgur.com/vMBrBP3.jpg)
