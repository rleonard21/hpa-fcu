# HPA Fire Control Unit
### Todo
* use a hardware timer and interrupt to put device to sleep
* reset the timer when the trigger ISR is handled
* when the timer reaches the set value, the associated ISR is handled
* associated ISR should put device into power-down mode and disable the counter
* only interrupts can wake the device from power-down mode