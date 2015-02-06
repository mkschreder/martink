#include <arch/soc.h>

#include "enc28j60.h"

// ENC28J60 Control Registers
// Control register definitions are a combination of address,
// bank number, and Ethernet/MAC/PHY indicator bits.
// - Register address        (bits 0-4)
// - Bank number        (bits 5-6)
// - MAC/PHY indicator        (bit 7)
#define ADDR_MASK        0x1F
#define BANK_MASK        0x60
#define SPRD_MASK        0x80
// All-bank registers
#define EIE              0x1B
#define EIR              0x1C
#define ESTAT            0x1D
#define ECON2            0x1E
#define ECON1            0x1F
// Bank 0 registers
#define ERDPTL           (0x00|0x00)
#define ERDPTH           (0x01|0x00)
#define EWRPTL           (0x02|0x00)
#define EWRPTH           (0x03|0x00)
#define ETXSTL           (0x04|0x00)
#define ETXSTH           (0x05|0x00)
#define ETXNDL           (0x06|0x00)
#define ETXNDH           (0x07|0x00)
#define ERXSTL           (0x08|0x00)
#define ERXSTH           (0x09|0x00)
#define ERXNDL           (0x0A|0x00)
#define ERXNDH           (0x0B|0x00)
#define ERXRDPTL         (0x0C|0x00)
#define ERXRDPTH         (0x0D|0x00)
#define ERXWRPTL         (0x0E|0x00)
#define ERXWRPTH         (0x0F|0x00)
#define EDMASTL          (0x10|0x00)
#define EDMASTH          (0x11|0x00)
#define EDMANDL          (0x12|0x00)
#define EDMANDH          (0x13|0x00)
#define EDMADSTL         (0x14|0x00)
#define EDMADSTH         (0x15|0x00)
#define EDMACSL          (0x16|0x00)
#define EDMACSH          (0x17|0x00)
// Bank 1 registers
#define EHT0             (0x00|0x20)
#define EHT1             (0x01|0x20)
#define EHT2             (0x02|0x20)
#define EHT3             (0x03|0x20)
#define EHT4             (0x04|0x20)
#define EHT5             (0x05|0x20)
#define EHT6             (0x06|0x20)
#define EHT7             (0x07|0x20)
#define EPMM0            (0x08|0x20)
#define EPMM1            (0x09|0x20)
#define EPMM2            (0x0A|0x20)
#define EPMM3            (0x0B|0x20)
#define EPMM4            (0x0C|0x20)
#define EPMM5            (0x0D|0x20)
#define EPMM6            (0x0E|0x20)
#define EPMM7            (0x0F|0x20)
#define EPMCSL           (0x10|0x20)
#define EPMCSH           (0x11|0x20)
#define EPMOL            (0x14|0x20)
#define EPMOH            (0x15|0x20)
#define EWOLIE           (0x16|0x20)
#define EWOLIR           (0x17|0x20)
#define ERXFCON          (0x18|0x20)
#define EPKTCNT          (0x19|0x20)
// Bank 2 registers
#define MACON1           (0x00|0x40|0x80)
#define MACON2           (0x01|0x40|0x80)
#define MACON3           (0x02|0x40|0x80)
#define MACON4           (0x03|0x40|0x80)
#define MABBIPG          (0x04|0x40|0x80)
#define MAIPGL           (0x06|0x40|0x80)
#define MAIPGH           (0x07|0x40|0x80)
#define MACLCON1         (0x08|0x40|0x80)
#define MACLCON2         (0x09|0x40|0x80)
#define MAMXFLL          (0x0A|0x40|0x80)
#define MAMXFLH          (0x0B|0x40|0x80)
#define MAPHSUP          (0x0D|0x40|0x80)
#define MICON            (0x11|0x40|0x80)
#define MICMD            (0x12|0x40|0x80)
#define MIREGADR         (0x14|0x40|0x80)
#define MIWRL            (0x16|0x40|0x80)
#define MIWRH            (0x17|0x40|0x80)
#define MIRDL            (0x18|0x40|0x80)
#define MIRDH            (0x19|0x40|0x80)
// Bank 3 registers
#define MAADR1           (0x00|0x60|0x80)
#define MAADR0           (0x01|0x60|0x80)
#define MAADR3           (0x02|0x60|0x80)
#define MAADR2           (0x03|0x60|0x80)
#define MAADR5           (0x04|0x60|0x80)
#define MAADR4           (0x05|0x60|0x80)
#define EBSTSD           (0x06|0x60)
#define EBSTCON          (0x07|0x60)
#define EBSTCSL          (0x08|0x60)
#define EBSTCSH          (0x09|0x60)
#define MISTAT           (0x0A|0x60|0x80)
#define EREVID           (0x12|0x60)
#define ECOCON           (0x15|0x60)
#define EFLOCON          (0x17|0x60)
#define EPAUSL           (0x18|0x60)
#define EPAUSH           (0x19|0x60)
// PHY registers
#define PHCON1           0x00
#define PHSTAT1          0x01
#define PHHID1           0x02
#define PHHID2           0x03
#define PHCON2           0x10
#define PHSTAT2          0x11
#define PHIE             0x12
#define PHIR             0x13
#define PHLCON           0x14

// ENC28J60 ERXFCON Register Bit Definitions
#define ERXFCON_UCEN     0x80
#define ERXFCON_ANDOR    0x40
#define ERXFCON_CRCEN    0x20
#define ERXFCON_PMEN     0x10
#define ERXFCON_MPEN     0x08
#define ERXFCON_HTEN     0x04
#define ERXFCON_MCEN     0x02
#define ERXFCON_BCEN     0x01
// ENC28J60 EIE Register Bit Definitions
#define EIE_INTIE        0x80
#define EIE_PKTIE        0x40
#define EIE_DMAIE        0x20
#define EIE_LINKIE       0x10
#define EIE_TXIE         0x08
#define EIE_WOLIE        0x04
#define EIE_TXERIE       0x02
#define EIE_RXERIE       0x01
// ENC28J60 EIR Register Bit Definitions
#define EIR_PKTIF        0x40
#define EIR_DMAIF        0x20
#define EIR_LINKIF       0x10
#define EIR_TXIF         0x08
#define EIR_WOLIF        0x04
#define EIR_TXERIF       0x02
#define EIR_RXERIF       0x01
// ENC28J60 ESTAT Register Bit Definitions
#define ESTAT_INT        0x80
#define ESTAT_LATECOL    0x10
#define ESTAT_RXBUSY     0x04
#define ESTAT_TXABRT     0x02
#define ESTAT_CLKRDY     0x01
// ENC28J60 ECON2 Register Bit Definitions
#define ECON2_AUTOINC    0x80
#define ECON2_PKTDEC     0x40
#define ECON2_PWRSV      0x20
#define ECON2_VRPS       0x08
// ENC28J60 ECON1 Register Bit Definitions
#define ECON1_TXRST      0x80
#define ECON1_RXRST      0x40
#define ECON1_DMAST      0x20
#define ECON1_CSUMEN     0x10
#define ECON1_TXRTS      0x08
#define ECON1_RXEN       0x04
#define ECON1_BSEL1      0x02
#define ECON1_BSEL0      0x01
// ENC28J60 MACON1 Register Bit Definitions
#define MACON1_LOOPBK    0x10
#define MACON1_TXPAUS    0x08
#define MACON1_RXPAUS    0x04
#define MACON1_PASSALL   0x02
#define MACON1_MARXEN    0x01
// ENC28J60 MACON2 Register Bit Definitions
#define MACON2_MARST     0x80
#define MACON2_RNDRST    0x40
#define MACON2_MARXRST   0x08
#define MACON2_RFUNRST   0x04
#define MACON2_MATXRST   0x02
#define MACON2_TFUNRST   0x01
// ENC28J60 MACON3 Register Bit Definitions
#define MACON3_PADCFG2   0x80
#define MACON3_PADCFG1   0x40
#define MACON3_PADCFG0   0x20
#define MACON3_TXCRCEN   0x10
#define MACON3_PHDRLEN   0x08
#define MACON3_HFRMLEN   0x04
#define MACON3_FRMLNEN   0x02
#define MACON3_FULDPX    0x01
// ENC28J60 MICMD Register Bit Definitions
#define MICMD_MIISCAN    0x02
#define MICMD_MIIRD      0x01
// ENC28J60 MISTAT Register Bit Definitions
#define MISTAT_NVALID    0x04
#define MISTAT_SCAN      0x02
#define MISTAT_BUSY      0x01
// ENC28J60 PHY PHCON1 Register Bit Definitions
#define PHCON1_PRST      0x8000
#define PHCON1_PLOOPBK   0x4000
#define PHCON1_PPWRSV    0x0800
#define PHCON1_PDPXMD    0x0100
// ENC28J60 PHY PHSTAT1 Register Bit Definitions
#define PHSTAT1_PFDPX    0x1000
#define PHSTAT1_PHDPX    0x0800
#define PHSTAT1_LLSTAT   0x0004
#define PHSTAT1_JBSTAT   0x0002
// ENC28J60 PHY PHCON2 Register Bit Definitions
#define PHCON2_FRCLINK   0x4000
#define PHCON2_TXDIS     0x2000
#define PHCON2_JABBER    0x0400
#define PHCON2_HDLDIS    0x0100

// ENC28J60 Packet Control Byte Bit Definitions
#define PKTCTRL_PHUGEEN  0x08
#define PKTCTRL_PPADEN   0x04
#define PKTCTRL_PCRCEN   0x02
#define PKTCTRL_POVERRIDE 0x01

// SPI operation codes
#define ENC28J60_READ_CTRL_REG       0x00
#define ENC28J60_READ_BUF_MEM        0x3A
#define ENC28J60_WRITE_CTRL_REG      0x40
#define ENC28J60_WRITE_BUF_MEM       0x7A
#define ENC28J60_BIT_FIELD_SET       0x80
#define ENC28J60_BIT_FIELD_CLR       0xA0
#define ENC28J60_SOFT_RESET          0xFF


// The RXSTART_INIT should be zero. See Rev. B4 Silicon Errata
// buffer boundaries applied to internal 8K ram
// the entire available packet buffer space is allocated
//
// start with recbuf at 0/
#define RXSTART_INIT     0x0
// receive buffer end
#define RXSTOP_INIT      (0x1FFF-0x0600-1)
// start TX buffer at 0x1FFF-0x0600, pace for one full ethernet frame (~1500 bytes)
#define TXSTART_INIT     (0x1FFF-0x0600)
// stp TX buffer at end of mem
#define TXSTOP_INIT      0x1FFF
// max frame length which the conroller will accept:
#define        MAX_FRAMELEN        1500        // (note: maximum ethernet frame length would be 1518)

// default mac. (A random D-link mac) 
const eth_mac_t default_mac = {0x00, 0x05, 0x5D, 0x12, 0x43, 0x85};

#define CS_LO(self) pio_write_pin(self->port, self->cs_pin, 0)
// set CS to 1 = passive
#define CS_HI(self) pio_write_pin(self->port, self->cs_pin, 1)

#define enc28j60_writereadbyte(self, ch) (\
	serial_putc(self->serial, ch),\
	serial_getc(self->serial)\
)

static uint8_t _enc28j60_read_reg(struct enc28j60 *self, uint8_t op, uint8_t address)
{
   CS_LO(self);
   // issue read command
   enc28j60_writereadbyte(self, op | (address & ADDR_MASK));
   uint8_t result = enc28j60_writereadbyte(self, 0x00); 
   
   // do dummy read if needed (for mac and mii, see datasheet page 29)
   if(address & 0x80)
   {
     result = enc28j60_writereadbyte(self, 0x00); 
   }
   // release CS
   CS_HI(self);
   return result;
}

static void _enc28j60_write_reg(struct enc28j60 *self, uint8_t op, uint8_t address, uint8_t data)
{
   CS_LO(self);
   // issue write command
   enc28j60_writereadbyte(self, op | (address & ADDR_MASK)); 
   enc28j60_writereadbyte(self, data); 
   
   CS_HI(self);
}

static void _enc28j60_read_buffer(struct enc28j60 *self, uint8_t* data, uint16_t len)
{
   CS_LO(self);
   // issue read command
   enc28j60_writereadbyte(self, ENC28J60_READ_BUF_MEM); 
   while(len)
   {
      len--;
      // read data
      *data = enc28j60_writereadbyte(self, 0x00); 
      data++;
   }
   *data='\0';
   CS_HI(self);
}

static void _enc28j60_write_buffer(struct enc28j60 *self, const uint8_t* data, uint16_t len)
{
   CS_LO(self);
   // issue write command
   enc28j60_writereadbyte(self, ENC28J60_WRITE_BUF_MEM);
   while(len)
   {
      len--;
      // write data
      enc28j60_writereadbyte(self, *data); 
      data++;
   }
   CS_HI(self);
}

static void _enc28j60_set_bank(struct enc28j60 *self, uint8_t address)
{
   // set the bank (if needed)
   if((address & BANK_MASK) != self->bank)
   {
      // set the bank
      _enc28j60_write_reg(self, ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
      _enc28j60_write_reg(self, ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
      self->bank = (address & BANK_MASK);
   }
}

static uint8_t _enc28j60_read_from(struct enc28j60 *self, uint8_t address)
{
   // set the bank
   _enc28j60_set_bank(self, address);
   // do the read
   return _enc28j60_read_reg(self, ENC28J60_READ_CTRL_REG, address);
}

static void _enc28j60_write_to(struct enc28j60 *self, uint8_t address, uint8_t data)
{
   // set the bank
   _enc28j60_set_bank(self, address);
   // do the write
   _enc28j60_write_reg(self, ENC28J60_WRITE_CTRL_REG, address, data);
}

static void _enc28j60_phy_write(struct enc28j60 *self, uint8_t address, uint16_t data) {
   // set the PHY register address
   _enc28j60_write_to(self, MIREGADR, address);
   // write the PHY data
   _enc28j60_write_to(self, MIWRL, data);
   _enc28j60_write_to(self, MIWRH, data>>8);
   // wait until the PHY write completes
   while(_enc28j60_read_from(self, MISTAT) & MISTAT_BUSY)
   {
      delay_us(1);
   }
}


//**************************************************************************************
// Flash the 2 RJ45 LEDs twice to show that the interface works
/*static void _enc68j60_flash_leds (struct enc28j60 *self)
{
	// LEDA=green LEDB=yellow
	//
	// 0x880 is PHLCON LEDB=on, LEDA=on
	// enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
	_enc28j60_phy_write(self, PHLCON,0x880);
	delay_us(500000L);
	//
	// 0x990 is PHLCON LEDB=off, LEDA=off
	// enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
	_enc28j60_phy_write(self, PHLCON,0x990);
	delay_us(500000L);
	//
	// 0x880 is PHLCON LEDB=on, LEDA=on
	// enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
	_enc28j60_phy_write(self, PHLCON,0x880);
	delay_us(500000L);
	//
	// 0x990 is PHLCON LEDB=off, LEDA=off
	// enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
	_enc28j60_phy_write(self, PHLCON,0x990);
	delay_us(500000L);
	//
   // 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
   // enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
   _enc28j60_phy_write(self, PHLCON,0x476);
	delay_us(100000L);
}
*/
void enc28j60_set_mac_addr(struct enc28j60 *self, const eth_mac_t macaddr){
	// write MAC address
	// NOTE: MAC address in ENC28J60 is byte-backward
	_enc28j60_write_to(self, MAADR5, macaddr[0]);
	_enc28j60_write_to(self, MAADR4, macaddr[1]);
	_enc28j60_write_to(self, MAADR3, macaddr[2]);
	_enc28j60_write_to(self, MAADR2, macaddr[3]);
	_enc28j60_write_to(self, MAADR1, macaddr[4]);
	_enc28j60_write_to(self, MAADR0, macaddr[5]);
}

//*****************************************************************************
void enc28j60_init(struct enc28j60 *self, serial_dev_t serial,
	pio_dev_t port, gpio_pin_t cs_pin)
{
	self->serial = serial;
	self->cs_pin = cs_pin;
	self->port = port;
	
	// initialize I/O
	// cs as output:
	pio_configure_pin(port, cs_pin, GP_OUTPUT);
	
	//ENC28J60_CONTROL_DDR |= (1 << ENC28J60_CONTROL_CS);
	CS_HI(self); // ss=0

	// perform system reset
	_enc28j60_write_reg(self, ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	delay_us(50000L);
	// check CLKRDY bit to see if reset is complete
	// The CLKRDY does not work. See Rev. B4 Silicon Errata point. Just wait.
	//while(!(enc28j60Read(ESTAT) & ESTAT_CLKRDY));
	// do bank 0 stuff
	// initialize receive buffer
	// 16-bit transfers, must write low byte first
	// set receive buffer start address
	self->next_packet_ptr = RXSTART_INIT;
	// Rx start
	_enc28j60_write_to(self, ERXSTL, RXSTART_INIT & 0xFF);
	_enc28j60_write_to(self, ERXSTH, RXSTART_INIT >> 8);
	// set receive pointer address
	_enc28j60_write_to(self, ERXRDPTL, RXSTART_INIT & 0xFF);
	_enc28j60_write_to(self, ERXRDPTH, RXSTART_INIT >> 8);
	// RX end
	_enc28j60_write_to(self, ERXNDL, RXSTOP_INIT & 0xFF);
	_enc28j60_write_to(self, ERXNDH, RXSTOP_INIT >> 8);
	// TX start
	_enc28j60_write_to(self, ETXSTL, TXSTART_INIT & 0xFF);
	_enc28j60_write_to(self, ETXSTH, TXSTART_INIT >> 8);
	// TX end
	_enc28j60_write_to(self, ETXNDL, TXSTOP_INIT & 0xFF);
	_enc28j60_write_to(self, ETXNDH, TXSTOP_INIT >> 8);
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
	_enc28j60_write_to(self, ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN);
	_enc28j60_write_to(self, EPMM0, 0x3f);
	_enc28j60_write_to(self, EPMM1, 0x30);
	_enc28j60_write_to(self, EPMCSL, 0xf9);
	_enc28j60_write_to(self, EPMCSH, 0xf7);
	//
	//
	// do bank 2 stuff
	// enable MAC receive
	_enc28j60_write_to(self, MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
	// bring MAC out of reset
	_enc28j60_write_to(self, MACON2, 0x00);
	// enable automatic padding to 60bytes and CRC operations
	_enc28j60_write_reg(self, ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
	// set inter-frame gap (non-back-to-back)
	_enc28j60_write_to(self, MAIPGL, 0x12);
	_enc28j60_write_to(self, MAIPGH, 0x0C);
	// set inter-frame gap (back-to-back)
	_enc28j60_write_to(self, MABBIPG, 0x12);
	// Set the maximum packet size which the controller will accept
	// Do not send packets longer than MAX_FRAMELEN:
	_enc28j60_write_to(self, MAMXFLL, MAX_FRAMELEN & 0xFF);	
	_enc28j60_write_to(self, MAMXFLH, MAX_FRAMELEN >> 8);
	// do bank 3 stuff

	enc28j60_set_mac_addr(self, default_mac);
	
	// no loopback of transmitted frames
	_enc28j60_phy_write(self, PHCON2, PHCON2_HDLDIS);
	// switch to bank 0
	_enc28j60_set_bank(self, ECON1);
	// enable interrutps
	_enc28j60_write_reg(self, ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
	// enable packet reception
	_enc28j60_write_reg(self, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

//*****************************************************************************
uint8_t enc28j60_read_version(struct enc28j60 *self)     // read the revision of the chip
{
	return(_enc28j60_read_from(self, EREVID));
}

//*****************************************************************************
void enc28j60_send(struct enc28j60 *self, const uint8_t* packet, uint16_t len)
{
	if (_enc28j60_read_from(self, EIR) & EIR_TXERIF) {
			_enc28j60_write_reg(self, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
			_enc28j60_write_reg(self, ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
			_enc28j60_write_reg(self, ENC28J60_BIT_FIELD_CLR, EIR, EIR_TXERIF);
	}
 
	// Set the write pointer to start of transmit buffer area
	_enc28j60_write_to(self, EWRPTL, TXSTART_INIT & 0xFF);
	_enc28j60_write_to(self, EWRPTH, TXSTART_INIT >> 8);
	// Set the TXND pointer to correspond to the packet size given
	_enc28j60_write_to(self, ETXNDL, (TXSTART_INIT + len) & 0xFF);
	_enc28j60_write_to(self, ETXNDH, (TXSTART_INIT + len) >> 8);
	// write per-packet control byte (0x00 means use macon3 settings)
	_enc28j60_write_reg(self, ENC28J60_WRITE_BUF_MEM, 0, 0x00);
	// copy the packet into the transmit buffer
	_enc28j60_write_buffer(self, packet, len);
	// send the contents of the transmit buffer onto the network
	_enc28j60_write_reg(self, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
   // Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
	/*if((_enc28j60_read_from(EIR) & EIR_TXERIF))
   {
      _enc28j60_write_reg(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
   }*/
}

//*****************************************************************************
// Gets a packet from the network receive buffer, if one is available.
// The packet will be headed by an ethernet header.
//      maxlen  The maximum acceptable length of a retrieved packet.
//      packet  Pointer where packet data should be stored.
// Returns: Packet length in bytes if a packet was retrieved, zero otherwise.
uint16_t enc28j60_recv(struct enc28j60 *self, uint8_t* packet, uint16_t maxlen)
{
	uint16_t rxstat;
	uint16_t len;
	// check if a packet has been received and buffered
	//if( !(enc28j60Read(EIR) & EIR_PKTIF) ){
        // The above does not work. See Rev. B4 Silicon Errata point 6.
	if( _enc28j60_read_from(self, EPKTCNT) ==0 )
   {
		return(0);
   }

	// Set the read pointer to the start of the received packet
	_enc28j60_write_to(self, ERDPTL, self->next_packet_ptr);
	_enc28j60_write_to(self, ERDPTH, self->next_packet_ptr >> 8);
	
   // read the next packet pointer
	self->next_packet_ptr  = _enc28j60_read_reg(self, ENC28J60_READ_BUF_MEM, 0);
	self->next_packet_ptr |= _enc28j60_read_reg(self, ENC28J60_READ_BUF_MEM, 0) << 8;

	// read the packet length (see datasheet page 43)
	len  = _enc28j60_read_reg(self, ENC28J60_READ_BUF_MEM, 0);
	len |= _enc28j60_read_reg(self, ENC28J60_READ_BUF_MEM, 0) << 8;
   len -= 4; //remove the CRC count
	
   // read the receive status (see datasheet page 43)
	rxstat  = _enc28j60_read_reg(self, ENC28J60_READ_BUF_MEM, 0);
	rxstat |= _enc28j60_read_reg(self, ENC28J60_READ_BUF_MEM, 0) << 8;

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
      _enc28j60_read_buffer(self, packet, len);
   }
   
   // Move the RX read pointer to the start of the next received packet
	// This frees the memory we just read out
	_enc28j60_write_to(self, ERXRDPTL, self->next_packet_ptr);
	_enc28j60_write_to(self, ERXRDPTH, self->next_packet_ptr >> 8);
	
   // decrement the packet counter indicate we are done with this packet
	_enc28j60_write_reg(self, ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
	return(len);
}

/*
static uint16_t 			_get(struct serial_interface *self){
	return SERIAL_NO_DATA; 
}

static uint16_t 			_put(struct serial_interface *self, uint8_t ch){
	return SERIAL_NO_DATA; 
}

static size_t				_putn(struct serial_interface *self, const uint8_t *data, size_t max_sz){
	struct enc28j60 *me = container_of(self, struct enc28j60, _serial_interface); 
	enc28j60_send(me, data, max_sz);
	return max_sz; 
}

static size_t				_getn(struct serial_interface *self, uint8_t *data, size_t max_sz){
	struct enc28j60 *me = container_of(self, struct enc28j60, _serial_interface); 
	return enc28j60_recv(me, data, max_sz); 
}

static void					_flush(struct serial_interface *self){
	
}

static size_t 				_waiting(struct serial_interface *self){
	return 1; 
}

struct serial_interface *enc28j60_get_serial_interface(struct enc28j60 *self){
	struct serial_interface *s = &self->_serial_interface;
	*s = (struct serial_interface){
		.put = _put,
		.get = _get,
		.putn = _putn,
		.getn = _getn,
		.flush = _flush,
		.waiting = _waiting
	}; 
	return s; 
}
*/
