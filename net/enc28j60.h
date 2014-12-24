#pragma once

#include "nettypes.h"

struct enc28j60 {
	serial_dev_t serial; // for serial com
	pio_dev_t port; // for cs pin
	gpio_pin_t cs_pin;

	uint16_t next_packet_ptr; 
	uint8_t bank;

	// exported interfaces 
	struct serial_if *_ex_serial;
};


// functions
/*
uint8_t enc28j60_ReadOp(struct enc28j60 *self, uint8_t op, uint8_t address);
void enc28j60_WriteOp(struct enc28j60 *self, uint8_t op, uint8_t address, uint8_t data);
void enc28j60_ReadBuffer(struct enc28j60 *self, uint16_t len, uint8_t* data);
void enc28j60_WriteBuffer(struct enc28j60 *self, uint16_t len, uint8_t* data);
void enc28j60_SetBank(struct enc28j60 *self, uint8_t address);
uint8_t enc28j60_Read(struct enc28j60 *self, uint8_t address);
void enc28j60_Write(struct enc28j60 *self, uint8_t address, uint8_t data);
void enc28j60_PhyWrite(struct enc28j60 *self, uint8_t address, uint16_t data);
void enc28j60_clkout(struct enc28j60 *self, uint8_t clk);
void InitPhy (struct enc28j60 *self);
*/

void enc28j60_init(struct enc28j60 *self, serial_dev_t serial, pio_dev_t port, gpio_pin_t cs_pin);
void enc28j60_set_mac_addr(struct enc28j60 *self, const eth_mac_t macaddr); 
void enc28j60_send(struct enc28j60 *self, const uint8_t* packet, uint16_t size);
uint16_t enc28j60_recv(struct enc28j60 *self, uint8_t* packet, uint16_t maxlen);
uint8_t enc28j60_read_version(struct enc28j60 *self);

struct serial_interface *enc28j60_get_serial_interface(struct enc28j60 *self);
