/*
	martink firmware project is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder

	Special thanks to:
	* Davide Gironi, original implementation
*/

#pragma once

/*
//setup port
#define DHT_DDR DDRD
#define DHT_PORT PORTD
#define DHT_PIN PIND
#define DHT_INPUTPIN PD2
*/

//sensor type
#define DHT_DHT11 1
#define DHT_DHT22 2
#define DHT_TYPE DHT_DHT11

//enable decimal precision (float)
#if DHT_TYPE == DHT_DHT11
#define DHT_FLOAT 0
#elif DHT_TYPE == DHT_DHT22
#define DHT_FLOAT 1
#endif

//timeout retries
#define DHT_TIMEOUT 200

#ifdef __cplusplus
extern "C" {
#endif

struct dht {
	pio_dev_t gpio;
	gpio_pin_t signal_pin;
	uint8_t sensor_type; 
};

void dht_init(struct dht *self,
	pio_dev_t gpio, gpio_pin_t signal_pin, uint8_t sensor_type); 
int8_t dht_read(struct dht *self, int8_t *temperature, int8_t *humidity);

#ifdef __cplusplus
}
#endif
