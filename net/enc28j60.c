#include <arch/soc.h>

#include "enc28j60.h"

static uint8_t enc28j60_Bank;
static uint16_t NextPacketPtr;
/*
#define ENC28J60_CONTROL_PORT    PORTD
#define ENC28J60_CONTROL_DDR     DDRD
#define ENC28J60_CONTROL_CS      PORTD2
*/
#ifndef CONFIG_ENC28J60_CS_PIN
#warning "ENC28J60 CS pin undefined!"
#define CONFIG_ENC28J60_CS_PIN GPIO_NONE
#endif

//#define ENC28J60_CONTROL_SO      PORTB5
//#define ENC28J60_CONTROL_SI      PORTB6
//#define ENC28J60_CONTROL_SCK     PORTB7
/*
// set CS to 0 = active
#define CSACTIVE ENC28J60_CONTROL_PORT &= ~(1 << ENC28J60_CONTROL_CS)
// set CS to 1 = passive
#define CSPASSIVE ENC28J60_CONTROL_PORT |= (1 << ENC28J60_CONTROL_CS)*/
#define CSACTIVE gpio_write_pin(CONFIG_ENC28J60_CS_PIN, 0)
// set CS to 1 = passive
#define CSPASSIVE gpio_write_pin(CONFIG_ENC28J60_CS_PIN, 1)

#undef spi_writereadbyte

#define spi_writereadbyte(ch) (__spi0_putc__(0, ch), __spi0_getc__(0))

//*****************************************************************************
uint8_t enc28j60_ReadOp(uint8_t op, uint8_t address)
{
   CSACTIVE;
   // issue read command
   spi_writereadbyte(op | (address & ADDR_MASK));
   uint8_t result = spi_writereadbyte(0x00); 
   
   // do dummy read if needed (for mac and mii, see datasheet page 29)
   if(address & 0x80)
   {
     result = spi_writereadbyte(0x00); 
   }
   // release CS
   CSPASSIVE;
   return result;
}

//*****************************************************************************
void enc28j60_WriteOp(uint8_t op, uint8_t address, uint8_t data)
{
   CSACTIVE;
   // issue write command
   spi_writereadbyte(op | (address & ADDR_MASK)); 
   spi_writereadbyte(data); 
   
   CSPASSIVE;
}

//*****************************************************************************
void enc28j60_ReadBuffer(uint16_t len, uint8_t* data)
{
   CSACTIVE;
   // issue read command
   spi_writereadbyte(ENC28J60_READ_BUF_MEM); 
   while(len)
   {
      len--;
      // read data
      *data = spi_writereadbyte(0x00); 
      data++;
   }
   *data='\0';
   CSPASSIVE;
}

//*****************************************************************************
void enc28j60_WriteBuffer(uint16_t len, uint8_t* data)
{
   CSACTIVE;
   // issue write command
   spi_writereadbyte(ENC28J60_WRITE_BUF_MEM);
   while(len)
   {
      len--;
      // write data
      spi_writereadbyte(*data); 
      data++;
   }
   CSPASSIVE;
}

//*****************************************************************************
void enc28j60_SetBank(uint8_t address)
{
   // set the bank (if needed)
   if((address & BANK_MASK) != enc28j60_Bank)
   {
      // set the bank
      enc28j60_WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
      enc28j60_WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
      enc28j60_Bank = (address & BANK_MASK);
   }
}

//*****************************************************************************
uint8_t enc28j60_Read(uint8_t address)
{
   // set the bank
   enc28j60_SetBank(address);
   // do the read
   return enc28j60_ReadOp(ENC28J60_READ_CTRL_REG, address);
}

//*****************************************************************************
void enc28j60_Write(uint8_t address, uint8_t data)
{
   // set the bank
   enc28j60_SetBank(address);
   // do the write
   enc28j60_WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);
}

//*****************************************************************************
void enc28j60_PhyWrite(uint8_t address, uint16_t data)
{
   // set the PHY register address
   enc28j60_Write(MIREGADR, address);
   // write the PHY data
   enc28j60_Write(MIWRL, data);
   enc28j60_Write(MIWRH, data>>8);
   // wait until the PHY write completes
   while(enc28j60_Read(MISTAT) & MISTAT_BUSY)
   {
      delay_us(15);
   }
}


//**************************************************************************************
// Flash the 2 RJ45 LEDs twice to show that the interface works
void InitPhy (void)
{
	/* Magjack leds configuration, see enc28j60 datasheet, page 11 */
	// LEDA=green LEDB=yellow
	//
	// 0x880 is PHLCON LEDB=on, LEDA=on
	// enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
	enc28j60_PhyWrite(PHLCON,0x880);
	delay_us(500000L);
	//
	// 0x990 is PHLCON LEDB=off, LEDA=off
	// enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
	enc28j60_PhyWrite(PHLCON,0x990);
	delay_us(500000L);
	//
	// 0x880 is PHLCON LEDB=on, LEDA=on
	// enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
	enc28j60_PhyWrite(PHLCON,0x880);
	delay_us(500000L);
	//
	// 0x990 is PHLCON LEDB=off, LEDA=off
	// enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
	enc28j60_PhyWrite(PHLCON,0x990);
	delay_us(500000L);
	//
   // 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
   // enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
   enc28j60_PhyWrite(PHLCON,0x476);
	delay_us(100000L);
}


//*****************************************************************************
void enc28j60_Init(uint8_t* macaddr)
{
   // initialize I/O
   // cs as output:
   gpio_configure(CONFIG_ENC28J60_CS_PIN, GP_OUTPUT); 
   //ENC28J60_CONTROL_DDR |= (1 << ENC28J60_CONTROL_CS);
   CSPASSIVE; // ss=0
   
   // perform system reset
   enc28j60_WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
   delay_us(50000L);
   // check CLKRDY bit to see if reset is complete
   // The CLKRDY does not work. See Rev. B4 Silicon Errata point. Just wait.
   //while(!(enc28j60Read(ESTAT) & ESTAT_CLKRDY));
   // do bank 0 stuff
   // initialize receive buffer
   // 16-bit transfers, must write low byte first
   // set receive buffer start address
   NextPacketPtr = RXSTART_INIT;
   // Rx start
   enc28j60_Write(ERXSTL, RXSTART_INIT & 0xFF);
   enc28j60_Write(ERXSTH, RXSTART_INIT >> 8);
   // set receive pointer address
   enc28j60_Write(ERXRDPTL, RXSTART_INIT & 0xFF);
   enc28j60_Write(ERXRDPTH, RXSTART_INIT >> 8);
   // RX end
   enc28j60_Write(ERXNDL, RXSTOP_INIT & 0xFF);
   enc28j60_Write(ERXNDH, RXSTOP_INIT >> 8);
   // TX start
   enc28j60_Write(ETXSTL, TXSTART_INIT & 0xFF);
   enc28j60_Write(ETXSTH, TXSTART_INIT >> 8);
   // TX end
   enc28j60_Write(ETXNDL, TXSTOP_INIT & 0xFF);
   enc28j60_Write(ETXNDH, TXSTOP_INIT >> 8);
   // do bank 1 stuff, packet filter:
   // For broadcast packets we allow only ARP packtets
   // All other packets should be unicast only for our mac (MAADR)
   //
   // The pattern to match on is therefore
   // Type     ETH.DST
   // ARP      BROADCAST
   // 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
   // in binary these poitions are:11 0000 0011 1111
   // This is hex 303F->EPMM0=0x3f,EPMM1=0x30
   enc28j60_Write(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN);
   enc28j60_Write(EPMM0, 0x3f);
   enc28j60_Write(EPMM1, 0x30);
   enc28j60_Write(EPMCSL, 0xf9);
   enc28j60_Write(EPMCSH, 0xf7);
   //
   //
   // do bank 2 stuff
   // enable MAC receive
   enc28j60_Write(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
   // bring MAC out of reset
   enc28j60_Write(MACON2, 0x00);
   // enable automatic padding to 60bytes and CRC operations
   enc28j60_WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
   // set inter-frame gap (non-back-to-back)
   enc28j60_Write(MAIPGL, 0x12);
   enc28j60_Write(MAIPGH, 0x0C);
   // set inter-frame gap (back-to-back)
   enc28j60_Write(MABBIPG, 0x12);
   // Set the maximum packet size which the controller will accept
   // Do not send packets longer than MAX_FRAMELEN:
   enc28j60_Write(MAMXFLL, MAX_FRAMELEN & 0xFF);	
   enc28j60_Write(MAMXFLH, MAX_FRAMELEN >> 8);
   // do bank 3 stuff
   // write MAC address
   // NOTE: MAC address in ENC28J60 is byte-backward
   enc28j60_Write(MAADR5, macaddr[0]);
   enc28j60_Write(MAADR4, macaddr[1]);
   enc28j60_Write(MAADR3, macaddr[2]);
   enc28j60_Write(MAADR2, macaddr[3]);
   enc28j60_Write(MAADR1, macaddr[4]);
   enc28j60_Write(MAADR0, macaddr[5]);
   // no loopback of transmitted frames
   enc28j60_PhyWrite(PHCON2, PHCON2_HDLDIS);
   // switch to bank 0
   enc28j60_SetBank(ECON1);
   // enable interrutps
   enc28j60_WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
   // enable packet reception
   enc28j60_WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

//*****************************************************************************
uint8_t enc28j60_getrev(void)     // read the revision of the chip
{
	return(enc28j60_Read(EREVID));
}

//*****************************************************************************
void enc28j60_PacketSend(uint16_t len, uint8_t* packet)
{
	if (enc28j60_Read(EIR) & EIR_TXERIF) {
			enc28j60_WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
			enc28j60_WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
			enc28j60_WriteOp(ENC28J60_BIT_FIELD_CLR, EIR, EIR_TXERIF);
	}
 
	// Set the write pointer to start of transmit buffer area
	enc28j60_Write(EWRPTL, TXSTART_INIT & 0xFF);
	enc28j60_Write(EWRPTH, TXSTART_INIT >> 8);
	// Set the TXND pointer to correspond to the packet size given
	enc28j60_Write(ETXNDL, (TXSTART_INIT + len) & 0xFF);
	enc28j60_Write(ETXNDH, (TXSTART_INIT + len) >> 8);
	// write per-packet control byte (0x00 means use macon3 settings)
	enc28j60_WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
	// copy the packet into the transmit buffer
	enc28j60_WriteBuffer(len, packet);
	// send the contents of the transmit buffer onto the network
	enc28j60_WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
   // Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
	/*if((enc28j60_Read(EIR) & EIR_TXERIF))
   {
      enc28j60_WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
   }*/
}

//*****************************************************************************
// Gets a packet from the network receive buffer, if one is available.
// The packet will be headed by an ethernet header.
//      maxlen  The maximum acceptable length of a retrieved packet.
//      packet  Pointer where packet data should be stored.
// Returns: Packet length in bytes if a packet was retrieved, zero otherwise.
uint16_t enc28j60_PacketReceive(uint16_t maxlen, uint8_t* packet)
{
	uint16_t rxstat;
	uint16_t len;
	// check if a packet has been received and buffered
	//if( !(enc28j60Read(EIR) & EIR_PKTIF) ){
        // The above does not work. See Rev. B4 Silicon Errata point 6.
	if( enc28j60_Read(EPKTCNT) ==0 )
   {
		return(0);
   }

	// Set the read pointer to the start of the received packet
	enc28j60_Write(ERDPTL, NextPacketPtr);
	enc28j60_Write(ERDPTH, NextPacketPtr >> 8);
	
   // read the next packet pointer
	NextPacketPtr  = enc28j60_ReadOp(ENC28J60_READ_BUF_MEM, 0);
	NextPacketPtr |= enc28j60_ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;

	// read the packet length (see datasheet page 43)
	len  = enc28j60_ReadOp(ENC28J60_READ_BUF_MEM, 0);
	len |= enc28j60_ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;
   len -= 4; //remove the CRC count
	
   // read the receive status (see datasheet page 43)
	rxstat  = enc28j60_ReadOp(ENC28J60_READ_BUF_MEM, 0);
	rxstat |= enc28j60_ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;

	// limit retrieve length
   if (len>maxlen-1)
   {
      len=maxlen-1;
   }
   // check CRC and symbol errors (see datasheet page 44, table 7-3):
   // The ERXFCON.CRCEN is set by default. Normally we should not
   // need to check this.
   if ((rxstat & 0x80)==0)
   {
      // invalid
      len=0;
   }
   else
   {
      // copy the packet from the receive buffer
      enc28j60_ReadBuffer(len, packet);
   }
   
   // Move the RX read pointer to the start of the next received packet
	// This frees the memory we just read out
	enc28j60_Write(ERXRDPTL, NextPacketPtr);
	enc28j60_Write(ERXRDPTH, NextPacketPtr >> 8);
	
   // decrement the packet counter indicate we are done with this packet
	enc28j60_WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
	return(len);
}
