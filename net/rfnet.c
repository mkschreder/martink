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

#include <avr/sleep.h>
#include <avr/power.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/crc16.h>
#include <util/delay.h>

#include "crypto/sha256.h"
//#include "random.h"
#include <radio/nrf24l01.h>
#include <arch/soc.h>

#include "rfnet.h"

#define RFNET_RETRY_COUNT 		20
#define RFNET_RETRY_TIMEOUT 	2000

// packet flags
#define PTF_SRT 0x10
#define PTF_ACK 0x20
#define PTF_FIN 0x40
#define PTF_RST 0x80


typedef enum {
	EV_FAIL, 
	EV_ENTER, 
	EV_LEAVE, 
	EV_PUT_DATA, 
	EV_GOT_REQUEST, 
	EV_GOT_RESPONSE, 
	EV_RETRIES_EXPENDED, 
	EV_TIMEOUT
} con_event_t; 

struct rfnet; 
#define CON_STATE(NAME, EV) \
	static uint8_t NAME(con_event_t EV)

typedef uint8_t (*rfnet_state_proc_t)(con_event_t ev);

struct rfnet {
	//struct rfnet_connection connection;
	rfnet_mac_t 				net_addr; 
	aes256_ctx_t 				enc_key; 
	struct packet				pbuf; 
	//rfnet_state_proc_t 	state; // current state of connection
	//timeout_t 					timeout; // time of the next timeout (used by state)
	//uint8_t							retries;
	
	rfnet_client_callback_proc_t 	cl_proc; 
	//rfnet_response_handler_proc_t cb_handle_response; 
	//rfnet_error_proc_t 						cb_error; 
}; 

static struct rfnet _net; 
static struct rfnet *net = &_net; 

#define TX_PIPE 0
#define RX_PIPE 1

typedef uint16_t nonce_t; 


static uint16_t gen_crc16(const uint8_t *data, uint16_t size)
{
	uint16_t crc = 0;
	for(int c = 0; c < size; c++){
		crc = _crc16_update(crc, *data++);
	}
	return crc; 
}

/// this function does nothing. 
static void ___validate_packet_sizes___(void){
	//struct packet p; 
	STATIC_ASSERT(sizeof(struct packet) == NRF24L01_PAYLOAD, "Packet size must be same as nrf payload size!");
	//STATIC_ASSERT(sizeof(struct packet) == sizeof(encrypted_packet_t), "Packet size must match aes encryption buffer size of 16 bytes");
	/*STATIC_ASSERT((((uint8_t*)&p.sum) - ((uint8_t*)&p)) !=
		(sizeof(struct packet) - sizeof(p.sum)),
			"Checksum field must be placed at the end of packet struct!");*/
	//STATIC_ASSERT(sizeof(struct packet) >= NRF24L01_PAYLOAD, "Transport layer block size must be same as packet size!"); 
}

static void packet_encrypt(aes256_ctx_t *ctx, struct packet *pack){
	// checksum is done on whole packet
	pack->crc = gen_crc16((uint8_t*)pack, sizeof(struct packet) - 
		sizeof(pack->crc));
	//memcpy(ep, pack, sizeof(struct packet));
	// encrypt the whole packet 
	// packets must always be sent fully encrypted
	//aes256_enc(ep, ctx);
	//aes256_enc(((uint8_t*)ep) + 16, ctx);
}

static uint8_t packet_decrypt(aes256_ctx_t *ctx, struct packet *pack){
	struct packet pp; 
	struct packet *p = &pp; 
	// dec() is destructive so we need to create local copy
	memcpy(p, pack, sizeof(struct packet)); 
	//aes256_dec(p, ctx);
	//aes256_dec(((uint8_t*)p) + 16, ctx);
	uint8_t valid = 0; 
	if(p->crc == gen_crc16((uint8_t*)p, sizeof(struct packet) - sizeof(p->crc)))
		valid = 1; 
	if(valid) memcpy(pack, p, sizeof(struct packet));
	return valid; 
}


void rfnet_init(volatile uint8_t *outport, volatile uint8_t *ddrport, uint8_t cepin, uint8_t cspin){
	
	// to avoid compiler warning
	___validate_packet_sizes___(); 
	
	// initialize the radio
	nrf24l01_init(outport, ddrport, cepin, cspin); 

	time_init(); 
}

void rfnet_configure(
		rfnet_mac_t addr, 
		const char *pw, 
		rfnet_client_callback_proc_t cb_proc){
			
	sha256_hash_t hash; 
	sha256(&hash, pw, strlen(pw)); 
	
	aes256_init(hash, &net->enc_key); 
	
	memcpy(net->net_addr, addr, sizeof(rfnet_mac_t)); 
	
	net->cl_proc = cb_proc; 
	
	// pipe 1 is always active as receive pipe
	nrf24l01_settxaddr(addr); 
}

void rfnet_process_events(void){
	// check for any received packets
	uint8_t pipe = 0xff; 
	
	//if(!net->state) return; 
	
	//poll radio for incoming packets
	if(nrf24l01_readready(&pipe) && pipe < 7) { 
		struct packet *pack = &net->pbuf; 
		
		//read buffer
		nrf24l01_read((uint8_t*)pack);
		
		//uart_printf(PSTR("GOT PACKET from %d!\n"), pipe); 
		
		if(!packet_decrypt(&net->enc_key, pack)){
			uart_puts("DECRFAIL!\n"); 
		} else {
			if(net->cl_proc){
				net->cl_proc(RFNET_EV_GOT_REQUEST, pack); 
			}
			
			pack->dst_id = pack->src_id; 
			pack->src_id = net->net_addr[4]; 
			pack->type = PTF_ACK; 
			
			packet_encrypt(&net->enc_key, &net->pbuf); 
			
			nrf24l01_settxaddr(net->net_addr);  
			nrf24l01_write((uint8_t*)&net->pbuf); 
		}
	}
	/*
	// process timeout events
	if(timeout_expired(net->timeout)){
		//rfnet_mac_t from; 
		net->state(EV_TIMEOUT); 
		net->timeout = timeout_from_now(RFNET_RETRY_TIMEOUT); 
		if(net->retries == 0)
			net->state(EV_RETRIES_EXPENDED); 
		else 
			net->retries--; 
	}*/
}

// sends a packet to the device id (will retry a number of times but then give up)
uint8_t rfnet_send(uint8_t device_id, uint8_t *data, uint8_t size){
	uint8_t s = (size > RFNET_PACKET_DATA_SIZE)?RFNET_PACKET_DATA_SIZE:size; 
	
	struct packet *pack = &net->pbuf; 
	pack->dst_id = device_id; 
	pack->src_id = net->net_addr[4]; 
	pack->type = PTF_SRT; 
	pack->size = size; 
	memcpy(&net->pbuf.data, data, s); 
	
	packet_encrypt(&net->enc_key, &net->pbuf); 
	
	nrf24l01_settxaddr(net->net_addr); 
	
	nrf24l01_write((uint8_t*)&net->pbuf); 
			
	return 0; 
}

uint8_t rfnet_is_busy(){
	//if(net->state != ST_IDLE) return 1; 
	return 0; 
}
