# HPA Fire Control Unit
### Todo
* use a hardware timer and interrupt to put device to sleep
* reset the timer when the trigger ISR is handled
* when the timer reaches the set value, the associated ISR is handled
* associated ISR should put device into power-down mode and disable the counter
* only interrupts can wake the device from power-down mode
* setting a timer for sleeping might not be necessary, and be better to
	go to sleep just immediately after the trigger ISR is handled

### Trigger ISR
* replace the busy-wait delay funciton with the 16-bit timer
* trigger ISR energizes the solenoid driver and starts the timer, exits
* timer overflow interrupt turns off the solenoid output, which can be
	done entirely in hardware if the driver is on pin OCxy
* timer overflow ISR turns off the timer and resets the counter start
