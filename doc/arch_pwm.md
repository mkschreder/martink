PWM peripheral interface
----------------------------
Martin K. Schröder | info@fortmax.se

Files:

* arch/manufacturer/model/pwm.h
* arch/pwm.h

Public interface
----------------

PWM interface is for controlling pwm hardware. Usually it would be built in, timer driven pwm channels, but the system does not limit you to just on board pwm channel You could just as easily have a driver for an i2c or spi pwm controller export this kind of interface and this would allow you to pass it to any component that requires a pwm interface (such as for example a motor speed controller). 

The basic pulse width modulator interface consists of following methods: 

| Interface method | Corresponding twi action |
|------------------|--------------------------|
uint16_t 		(*set_channel_duty)(struct pwm_interface *self, uint8_t channel, uint16_t value_us) | Sets channel "on" time to value specified in microseconds. A pwm controller can have several channels. Channel id is implementation specific id of the channel. returns actual duty cycle time that has been set. 
uint16_t 		(*set_channel_period)(struct pwm_interface *self, uint8_t channel, uint16_t value_us) | Occasionally it may be beneficial to change period time of the pwm signal. Implementation should pick the prescaler value that gives the closest match to the specified period. This does not guarantee that the period value will be exact. but implementation should try to make it as exact as possible. returns actual period that has been set. 


Implementation macro guidelines
----------------------

| Hardware method | Description |
|-----------------|-------------|
pwm0_enable() | enable pwm channel and configure it to settings most commonly seen in hobby controller context: ie pulse width between 0 and 2000 usec. The frequency is also kept as high as possible. Some servos may not be able to handle higher update rate but this can be fixed later if it becomes an issue. 
pwm0_set(speed) | sets pulse width in microseconds. 

Example usage
--------------------

pwm0_init();
pwm1_init();

pwm0_set(1500); // half throttle
pwm0_set(1000); // zero throttle
pwm0_set(2000); // full throttle
