/**
	This file is part of martink project.

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
*/

#pragma once

#include <avr/io.h>
#include "crypto/aes/aes.h"
//#include "stack.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char rfnet_mac_t[5]; 
typedef unsigned char rfnet_key_t[32]; 
typedef unsigned char rfnet_pw_t[13]; 

typedef int8_t 		rfnet_handle_t; 

#define RFNET_INVALID_HANDLE -1

#define RFNET_PACKET_DATA_SIZE (22)

typedef enum {
	RFNET_ERR_UNDEFINED = 0, 
	RFNET_ERR_READ_REQ_SEND_FAIL = -1, 
	RFNET_ERR_TIMEOUT 					 = -2, 
	RFNET_ERR_INVALID_MESSAGE 	 = -3, 
	RFNET_ERR_BUFFER_SEND_FAILED = -4,
	RFNET_ERR_NONCE_MISMATCH 		 = -5,
	RFNET_ERR_INVALID_PACKET		 = -6,
	RFNET_ERR_CONNECT_TIMEOUT 	 = -7,
} rfnet_error_t; 

typedef enum {
	RFNET_EV_GOT_REQUEST = 1, 
	RFNET_EV_GOT_RESPONSE, 
	RFNET_EV_GOT_BCAST_PACKET
} rfnet_event_t; 

struct __attribute__((__packed__)) packet {
	uint8_t 	dst_id; 
	uint8_t 	src_id; 
	uint8_t 	type; 
	uint32_t 	seq; // unique packet id
	uint8_t 	size; 
	uint8_t 	data[RFNET_PACKET_DATA_SIZE]; 
	uint16_t 	crc; // checksum
}; 

typedef uint8_t (*rfnet_client_callback_proc_t)(
	rfnet_event_t ev, 
	struct packet *pbuf); 
	
void rfnet_init(volatile uint8_t *outport, volatile uint8_t *ddrport, 
		uint8_t cepin, uint8_t cspin); 
void rfnet_configure(rfnet_mac_t addr, const char *root_pw, 
		rfnet_client_callback_proc_t proc); 
		
uint8_t rfnet_send(uint8_t device_id, uint8_t *data, uint8_t size); 
void rfnet_process_events(void); 
uint8_t rfnet_is_busy(void); 


#ifdef __cplusplus
}
#endif
