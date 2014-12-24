/*
pcf8574 lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef PCF8574_H_
#define PCF8574_H_

struct pcf8574 {
	i2c_dev_t i2c;
	uint8_t device_id;
	uint8_t in_reg, out_reg; 
};

void pcf8574_init(struct pcf8574 *self, i2c_dev_t i2c, uint8_t device_id);

uint8_t pcf8574_write_word(struct pcf8574 *self, uint8_t data);
uint8_t pcf8574_write_pin(struct pcf8574 *self, uint8_t pin, uint8_t value);
uint8_t pcf8574_read_word(struct pcf8574 *self);
uint8_t pcf8574_read_pin(struct pcf8574 *self, uint8_t pin);

//extern int8_t pcf8574_getoutput(uint8_t deviceid);
//extern int8_t pcf8574_getoutputpin(uint8_t deviceid, uint8_t pin);
//extern int8_t pcf8574_setoutputpinhigh(uint8_t deviceid, uint8_t pin);
//extern int8_t pcf8574_setoutputpinlow(uint8_t deviceid, uint8_t pin);
//extern int8_t pcf8574_setoutputpins(uint8_t deviceid, uint8_t pinstart, uint8_t pinlength, int8_t data);

#endif
