Analog to digital converters
----------------------------
Martin K. Schröder
info@fortmax.se

These devices are expected to provide a service of reading an analog signal on one of available pins and reporting the value as a digital 16 bit number. Adc implementation is limited to 256 channels (although can easily be extended in the future if one needs more). An adc driver should typically only control one type of device or a few types of devices that are very similar to each other. If a device differs too much, a separate driver should be created.

Public interface
----------------

* int16_t adc->read_pin(adc, pin_number) - reads adc pin and returns it's integer representation. 

Implementation macro guidelines
----------------------

* adc0_enable() - provides means to turn on the adc peripheral
* adc0_disable() - will turn off adc to save power
* adc0_init_default() - configures adc to default settings (typically internal vref, left alignment and slowest possible clock for highest accuracy)
* adc0_set_vref(adcref) - must provide means of setting voltage reference source for the conversion.
* adc0_set_prescaler(adc_clock) - sets adc clock prescaler 
* adc0_set_alignment(adc_align) - must provide means of setting alignment of result (left or right)
* adc0_interrupt_on() - turns on adc interrupt 
* adc0_interrupt_off() - turns off adc interrupt
* adc0_set_channel(adc_chan) - provides means to tell adc peripheral which channel to sample
* adc0_get_channel() - evaluates to whatever channel we are currently sampling
* adc0_start_conversion() - kicks off adc conversion
* adc0_conversion_in_progress() - checks if a conversion is currently in provress
* adc0_wait_for_completed_conversion() - returns when current conversion has finished
* adc0_read() (uint16_t) - simply waits for previous conversion to complete first and then returns sampled value for currently selected channel. 
* adc0_read_immediate(chan) - kicks off a conversion, waits for it to complete and returns the value for supplied channel. 
* adc0_set_mode(adc_mode) - provides access for setting different adc modes (such as manual or automatic continuous mode where data is sampled all the time and cached for future use. Implementation dependent..)

Macros should be provided for every peripheral separately if multiple devices are available - ie adc0, adc1 etc.. 

Typical direct usage
--------------------

```
adc0_init_default();
printf("ADC value: %d\n", adc0_read_immediate(channel_id));
```

For other options it is best to check out implementation specific file in arch/<manufacturer>/<cpu>/adc.h



