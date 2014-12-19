Timers and tick counters
----------------------------
Martin K. Schröder | info@fortmax.se

Files:

* arch/manufacturer/model/time.h
* arch/time.h

Intro
-----

It is often necessary for implementation to provide a timer/counter for counting cpu clock cycles because this is the only way to do fairly exact timing when interrupts are enabled. More complex chips usually have a dedicated timer for this and an instruction to read clock counter. But for architectures that do not have this functionality, one of the timers needs to be used.

Usage of clock counter
---------------

In the rest of the system clock counter value can be used in many ways - from simple delays to asynchronous events. If you disable this functionality then some drivers may stop working, but ideally they would not even compile. There is a special config field called CONFIG_TIMESTAMP_COUNTER that enables timestamping functionality and drivers that need this functionality should be configured in the corresponding KConfig file to depend on it being set. 

Public interface
----------------

Timer counter currently does not export interfaces. This may change when drivers would require an abstract timer object. For now, timers are mostly only used directly by drivers in the arch layer. While higher level drivers mostly only use timestamping to calculate delays and timeouts. 


Implementation guidelines
----------------------

| Hardware method | Description |
|-----------------|-------------|
timestamp_init() | sets up timestamping counter to start counting. This configures one of the hardware timers to run at a frequency that is preferably exactly 1Mhz. But it may be 2Mhz or 8Mhz.. the value is implementation specific and the timestamp value returned by this method is in ticks count. So you need to convert it to microseconds in order to see what it is in microseconds.
timestamp_now() | gets current timestamp value in ticks. The value is ticks from cpu start. but remember that the value will wrap around after about 70 minutes. So do not measure overly long delays using this counter. The standard use is to capture the counter, do something and then check whether certain amount of time has elapsed. Functions below help you do this. 
timestamp_ticks_to_us(ticks) | converts ticks value to microseconds. 
timestamp_us_to_ticks(us) | converts microseconds to equivalent number of ticks
timestamp_before(unknown, known) | checks if unknown is before known. This function is required to handle overflow of the ticks counter correctly. 
timestamp_after(a,b) | checks if a is after b, also overflow safe
timestamp_from_now_us(us) | calculates a timeout value a number of microseconds in the future
timestamp_expired(timeout) | checks if a previously set timeout has expired
timestamp_delay_us(timestamp_t usec) | equivalent to busy loop delay, but more exact when interrupts are enabled. 
delay_us(usec) | a generic method that can be pinted directly to timestamp_delay_us
timestamp_ticks_since(timestamp) | returns ticks that have elapsed from some timestamp

There are more timer hardware specific methods but they will not be covered here for now. Instead, have a look at implementation specific time.h file to get an idea. 

Example usage
--------------------

```
timestamp_init();
timeout_t timeout = timeout_from_now(1000000); // 1s timeout
while(1){
	// do work
	if(timeout_expired(timeout))
		break; // will stop after 1 second
}
```
