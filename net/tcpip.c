/** 
 * 	Maintainer: Martin K. Schröder 
 * 
 *  Thanks for the code from ip_arp_udp_tcp.c!
 * 
 *  Date: 2014
 * 
 * 	info@fortmax.se
 */

#include <arch/soc.h>

#include "net.h"
#include "enc28j60.h"
#include "tcpip.h"
#include <string.h>

static uint16_t Checksum(uint8_t *buf, uint16_t len, uint8_t type); 

void ip_set_ip(struct ip_network *self, eth_mac_t macaddr, ip_addr_t myip){
	memcpy(self->macaddr, macaddr, sizeof(eth_mac_t)); 
	memcpy(self->ipaddr, myip, sizeof(ip_addr_t)); 
}

//*****************************************************************************
// you must call this function once before you use any of the other functions:
void ip_init(struct ip_network *self, struct packet_interface *eth, ip_addr_t myip, uint8_t *buffer, uint16_t size)
{
	//enc28j60_Init(mymac);    //initialize enc28j60
	//delay_us(10000L);
	//InitPhy();
	
	self->seqnum = 0xa; 
	self->ip_identifier = 1; 
	self->info_data_len = 0; 
	self->info_hdr_len = 0; 
	self->port = 80; 
	self->buf = buffer; 
	self->max_buffer_size = size; 

	eth_mac_t default_mac = {0x00, 0x05, 0x5D, 0x12, 0x43, 0x85}; 
	ip_set_ip(self, default_mac, myip); 
}

void ip_send(struct ip_network *self){
	self->eth->read(self->eth, self->buf, self->len); 
	//enc28j60_PacketSend(self->len, self->buf); 
}

//*****************************************************************************
uint8_t _isArpPacketAddressedToUs(struct ip_network *self){
	if (self->len < 41){
		return(0);
	}
	if ((self->buf[ETH_TYPE_H_P] != ETHTYPE_ARP_H_V) || 
	(self->buf[ETH_TYPE_L_P] != ETHTYPE_ARP_L_V)){
		return(0);
	}
	if(memcmp(&self->buf[ETH_ARP_DST_IP_P], self->ipaddr, sizeof(ip_addr_t)) == 0)
		return 1; 
	return 0; 
}



//*****************************************************************************
uint8_t _isIPPacketAddressedToUs(struct ip_network *self){
	//eth+ip+udp header is 42
	if (self->len < 42){
		return(0);
	}
	if ((self->buf[ETH_TYPE_H_P] != ETHTYPE_IP_H_V) || 
	(self->buf[ETH_TYPE_L_P]!=ETHTYPE_IP_L_V))
	{
		return(0);
	}
	if (self->buf[IP_HEADER_LEN_VER_P] != 0x45)

	{
		// must be IP V4 and 20 byte header
		return(0);
	}

	if(memcmp(&self->buf[IP_DST_P], self->ipaddr, sizeof(ip_addr_t)) == 0)
	return 1;
	return 0; 
}


void ip_setDstAddr(struct ip_network *self, eth_mac_t mac, ip_addr_t ip){
	memcpy(&self->buf[IP_DST_IP_P], ip, sizeof(ip_addr_t)); 
	memcpy(&self->buf[IP_SRC_IP_P], self->ipaddr, sizeof(ip_addr_t)); 
	memcpy(&self->buf[ETH_DST_MAC], mac, sizeof(eth_mac_t)); 
	memcpy(&self->buf[ETH_SRC_MAC], self->macaddr, sizeof(eth_mac_t)); 
}

//*****************************************************************************
// make a return eth header from a received eth packet
// replace with setDstAddr(macaddr, buf[ETH_SRC_MAC])
void _initEthernet(struct ip_network *self, eth_mac_t dst_mac)
{
	memcpy(&self->buf[ETH_DST_MAC], dst_mac, sizeof(eth_mac_t)); 
	memcpy(&self->buf[ETH_SRC_MAC], self->macaddr, sizeof(eth_mac_t)); 
}



//*****************************************************************************
// make a new eth header for IP packet
// replace with setDstAddr(dst_mac, ??)
/*
void _initIPPacket(uint8_t* dst_mac)
{
	
   uint8_t i=0;

   //copy the destination mac from the source and fill my mac into src
   while(i < 6)

   {
      buf[ETH_DST_MAC + i] = dst_mac[i];
      buf[ETH_SRC_MAC + i] = macaddr[i];
      i++;
   }

   buf[ETH_TYPE_H_P] = ETHTYPE_IP_H_V;
   buf[ETH_TYPE_L_P] = ETHTYPE_IP_L_V;
}*/


//*****************************************************************************
static void _updateIPChecksum(struct ip_network *self)
{
   uint16_t ck;
   uint8_t *buf = self->buf; 
   
   // clear the 2 byte checksum
   buf[IP_CHECKSUM_P] = 0;
   buf[IP_CHECKSUM_P+1] = 0;
   buf[IP_FLAGS_P] = 0x40; // don't fragment
   buf[IP_FLAGS_P+1] = 0;  // fragement offset
   buf[IP_TTL_P] = 64; // ttl
   // calculate the checksum:
   ck = Checksum(&buf[IP_P], IP_HEADER_LEN, 0);
   buf[IP_CHECKSUM_P] = ck >> 8;
   buf[IP_CHECKSUM_P+1] = ck & 0xff;
}

//*****************************************************************************
// make a return ip header from a received ip packet
// buf[IP_SRC_P + i]
void _initIP(struct ip_network *self, ip_addr_t dst_ip, uint16_t totlen)
{
	uint8_t *buf = self->buf; 
	
	buf[ETH_TYPE_H_P] = ETHTYPE_IP_H_V;
	buf[ETH_TYPE_L_P] = ETHTYPE_IP_L_V;
	
	memcpy(self->buf + IP_DST_P, dst_ip, sizeof(ip_addr_t)); 
	memcpy(self->buf + IP_SRC_P, self->ipaddr, sizeof(ip_addr_t)); 
	
	buf[IP_P] = IP_V4_V | IP_HEADER_LENGTH_V;

	// set TOS to default 0x00
	buf[IP_TOS_P ] = 0x00;

	// set total length
	buf[IP_TOTLEN_H_P] = (totlen >> 8) & 0xff;
	buf[IP_TOTLEN_L_P] = totlen & 0xff;

	// set packet identification
	buf[IP_ID_H_P] = (self->ip_identifier >> 8) & 0xff;
	buf[IP_ID_L_P] = self->ip_identifier & 0xff;
	self->ip_identifier++;

	// set fragment flags	
	buf[IP_FLAGS_H_P] = 0x00;
	buf[IP_FLAGS_L_P] = 0x00;

	// set Time To Live
	buf[IP_TTL_P] = 128;

	_updateIPChecksum(self);
}

//*****************************************************************************
// make a new ip header for tcp packet
// make a return ip header from a received ip packet
void _initTCP(struct ip_network *self, uint16_t len, ip_addr_t dst_ip)
{
	uint8_t *buf = self->buf; 
	
	_initIP(self, dst_ip, len); 
	
	// set ip packettype to tcp/udp/icmp...
	buf[IP_PROTO_P] = IP_PROTO_TCP_V;
	
	_updateIPChecksum(self);
}

//*****************************************************************************
// make a return tcp header from a received tcp packet
// rel_ack_num is how much we must step the seq number received from the
// other side. We do not send more than 255 bytes of text (=data) in the tcp packet.
// If mss=1 then mss is included in the options list
//
// After calling this function you can fill in the first data byte at TCP_OPTIONS_P+4
// If cp_seq=0 then an initial sequence number is used (should be use in synack)
// otherwise it is copied from the packet we received
void _initTCPHeader(struct ip_network *self, uint16_t rel_ack_num, uint8_t mss, uint8_t cp_seq)
{
	uint8_t tseq;
	uint8_t *buf = self->buf; 

	memcpy(buf + TCP_DST_PORT_H_P, buf + TCP_SRC_PORT_H_P, 2); 

	// setup listen port
	buf[TCP_SRC_PORT_H_P] = self->port >> 8; 
	buf[TCP_SRC_PORT_L_P] = self->port & 0xff;

	uint8_t i = 4; 
	// sequence numbers:
	// add the rel ack num to SEQACK
	while(i > 0){
		rel_ack_num = buf[TCP_SEQ_H_P + i - 1] + rel_ack_num;
		tseq = buf[TCP_SEQACK_H_P + i - 1];
		buf[TCP_SEQACK_H_P + i - 1] = 0xff & rel_ack_num;
		if (cp_seq) {
			 // copy the acknum sent to us into the sequence number
			 buf[TCP_SEQ_H_P + i - 1] = tseq;
		}

		else {
			 buf[TCP_SEQ_H_P + i - 1] = 0; // some preset vallue
		}
		rel_ack_num = rel_ack_num >> 8;
		i--;
	}
	if (cp_seq == 0)

	{
		// put inital seq number
		buf[TCP_SEQ_H_P + 0] = 0;
		buf[TCP_SEQ_H_P + 1] = 0;
		// we step only the second byte, this allows us to send packts 
		// with 255 bytes or 512 (if we step the initial seqnum by 2)
		buf[TCP_SEQ_H_P + 2] = self->seqnum; 
		buf[TCP_SEQ_H_P + 3] = 0;
		// step the inititial seq num by something we will not use
		// during this tcp session:
		self->seqnum += 2;
	}
	// zero the checksum
	buf[TCP_CHECKSUM_H_P] = 0;
	buf[TCP_CHECKSUM_L_P] = 0;

	// The tcp header length is only a 4 bit field (the upper 4 bits).
	// It is calculated in units of 4 bytes. 
	// E.g 24 bytes: 24/4=6 => 0x60=header len field
	//buf[TCP_HEADER_LEN_P]=(((TCP_HEADER_LEN_PLAIN+4)/4)) <<4; // 0x60
	if (mss > 0)

	{
		// the only option we set is MSS to 1408:
		// 1408 in hex is 0x580
		buf[TCP_OPTIONS_P] = 2;
		buf[TCP_OPTIONS_P + 1] = 4;
		buf[TCP_OPTIONS_P + 2] = 0x05; 
		buf[TCP_OPTIONS_P + 3] = 0x80;
		// 24 bytes:
		buf[TCP_HEADER_LEN_P] = 0x60;
	}

	else

	{
		// no options:
		// 20 bytes:
		buf[TCP_HEADER_LEN_P] = 0x50;
	}
}

static void _initARP(struct ip_network *self, eth_mac_t dst_mac, ip_addr_t dst_ip){
	memcpy(self->buf + ETH_ARP_DST_MAC_P, dst_mac, sizeof(eth_mac_t)); 
	memcpy(self->buf + ETH_ARP_SRC_MAC_P, self->macaddr, sizeof(eth_mac_t)); 
	memcpy(self->buf + ETH_ARP_DST_IP_P, dst_ip, sizeof(ip_addr_t)); 
	memcpy(self->buf + ETH_ARP_SRC_IP_P, self->ipaddr, sizeof(ip_addr_t)); 
}

//*****************************************************************************
void _initARPResponse(struct ip_network *self)
{
	_initEthernet(self, self->buf + ETH_SRC_MAC); 
	//_initIP(buf + IP_SRC_P, 0);
	
	// set arp packet type
	self->buf[ETH_TYPE_H_P] = ETHTYPE_ARP_H_V;
	self->buf[ETH_TYPE_L_P] = ETHTYPE_ARP_L_V; 
	
	self->buf[ETH_ARP_OPCODE_H_P] = ETH_ARP_OPCODE_REPLY_H_V;
	self->buf[ETH_ARP_OPCODE_L_P] = ETH_ARP_OPCODE_REPLY_L_V;
	
	_initARP(self, self->buf + ETH_ARP_SRC_MAC_P, self->buf + ETH_ARP_SRC_IP_P); 
	
	self->len = 42; 
	
	// eth+arp is 42 bytes:
	//enc28j60_PacketSend(42,buf); 
}



//*****************************************************************************
void _initICMP(struct ip_network *self)
{
	uint8_t *buf = self->buf; 
	uint16_t totlen = ((uint16_t)buf[IP_TOTLEN_H_P] << 8) | buf[IP_TOTLEN_L_P]; 
	
	_initEthernet(self, buf + ETH_SRC_MAC);
	_initIP(self, buf + IP_SRC_P, totlen);

	buf[ICMP_TYPE_P] = ICMP_TYPE_ECHOREPLY_V;
	
	//_updateIPChecksum(); 
	if (buf[ICMP_CHECKSUM_H_P] > (0xFF-0x08))
		buf[ICMP_CHECKSUM_L_P]++;
	buf[ICMP_CHECKSUM_H_P] += 0x08;
    /*
	uint16_t ck = Checksum(buf + IP_HEADER_LEN + ETH_HEADER_LEN, 24, 0); 
	buf[ICMP_CHECKSUM_H_P] = ck >> 8; 
	buf[ICMP_CHECKSUM_L_P] = ck & 0xff; 
	*/
	//memset(buf + IP_HEADER_LEN + ETH_HEADER_LEN + 8, 0xdd, 10); 
	self->len = ETH_HEADER_LEN + totlen; 
	//
	//enc28j60_PacketSend(len, buf);
}



//*****************************************************************************
// you can send a max of 220 bytes of data
void _initUDPResponse(struct ip_network *self, char *data, uint8_t datalen, uint16_t port)
{
	uint16_t ck;
	uint8_t *buf = self->buf; 

	_initEthernet(self, buf + ETH_SRC_MAC);

	if (datalen > 220){
		datalen = 220;
	}

	_initIP(self, buf + IP_SRC_P, IP_HEADER_LEN + UDP_HEADER_LEN + datalen);

	buf[UDP_DST_PORT_H_P] = port >> 8;
	buf[UDP_DST_PORT_L_P] = port & 0xff;
	// source port does not matter and is what the sender used.
	// calculte the udp length:
	buf[UDP_LEN_H_P] = 0;
	buf[UDP_LEN_L_P] = UDP_HEADER_LEN + datalen;
	// zero the checksum
	buf[UDP_CHECKSUM_H_P] = 0;
	buf[UDP_CHECKSUM_L_P] = 0;

	memcpy(buf + UDP_DATA_P, data, datalen); 

	ck = Checksum(&buf[IP_SRC_P], 16 + datalen, 1);
	buf[UDP_CHECKSUM_H_P] = ck >> 8;
	buf[UDP_CHECKSUM_L_P] = ck & 0xff;

	self->len = UDP_HEADER_LEN + IP_HEADER_LEN + ETH_HEADER_LEN + datalen;
}

uint8_t _ipOnLan(struct ip_network *self, ip_addr_t src, ip_addr_t dst){
	if(src[0] == 0 || dst[0] == 0) {
		return 0;
	}
	for(int i = 0; i < 4; i++){
		if((src[i] & self->netmask[i]) != (dst[i] & self->netmask[i])) {
			return 0;
		}
	}
	return 1;
}

void ip_send_udp(struct ip_network *self, const uint8_t *data, uint8_t datalen, 
	uint16_t sport, eth_mac_t dmac, ip_addr_t dip, uint16_t dport) {
	
	uint8_t *buf = self->buf; 
	
	if (datalen>220)
			datalen = 220;
			
	uint16_t totlen = (IP_HEADER_LEN+UDP_HEADER_LEN+datalen); 
	
	_initEthernet(self, dmac); 
	
	_initIP(self, dip, totlen); 
	
	// see http://tldp.org/HOWTO/Multicast-HOWTO-2.html
	// multicast or broadcast address, https://github.com/jcw/ethercard/issues/59
	/*if ((dip[0] & 0xF0) == 0xE0 || *((unsigned long*) dip) == 0xFFFFFFFF)
			EtherCard::copyMac(buf + ETH_DST_MAC, allOnes);*/
	
	//memcpy_P(buf + IP_P,iphdr,9);
	
	buf[IP_PROTO_P] = IP_PROTO_UDP_V;
	
	buf[UDP_DST_PORT_H_P] = (dport>>8);
	buf[UDP_DST_PORT_L_P] = dport;
	buf[UDP_SRC_PORT_H_P] = (sport>>8);
	buf[UDP_SRC_PORT_L_P] = sport;
	buf[UDP_LEN_H_P] = 0;
	buf[UDP_CHECKSUM_H_P] = 0;
	buf[UDP_CHECKSUM_L_P] = 0;
	
	memcpy(buf + UDP_DATA_P, data, datalen);
	
	// total length field in the IP header must be set:
	
	_updateIPChecksum(self);
	
	buf[UDP_LEN_H_P] = (UDP_HEADER_LEN+datalen) >>8;
	buf[UDP_LEN_L_P] = UDP_HEADER_LEN+datalen;
	uint16_t ck = Checksum(&buf[IP_SRC_P], 16 + datalen, 1);
	buf[UDP_CHECKSUM_H_P] = ck >> 8;
	buf[UDP_CHECKSUM_L_P] = ck & 0xff;
   
	self->len = UDP_HEADER_LEN + IP_HEADER_LEN + ETH_HEADER_LEN + datalen; 
	
	ip_send(self); 
}

//*****************************************************************************
void _initTCPSynACK(struct ip_network *self)
{
	uint16_t ck;
	uint8_t *buf = self->buf; 

	_initEthernet(self, buf + ETH_SRC_MAC);

	_initIP(self, buf + IP_SRC_P, IP_HEADER_LEN + TCP_HEADER_LEN_PLAIN + 4);

	buf[TCP_FLAG_P] = TCP_FLAGS_SYNACK_V;
	
	_initTCPHeader(self, 1, 1, 0);
	
	// calculate the checksum, len=8 (start from ip.src) + TCP_HEADER_LEN_PLAIN + 4 (one option: mss)
	ck = Checksum(&buf[IP_SRC_P], 8 + TCP_HEADER_LEN_PLAIN + 4, 2);
	buf[TCP_CHECKSUM_H_P] = ck >> 8;
	buf[TCP_CHECKSUM_L_P] = ck & 0xff;
	
	// add 4 for option mss:
	self->len = IP_HEADER_LEN + TCP_HEADER_LEN_PLAIN + 4 + ETH_HEADER_LEN;
}



//*****************************************************************************
// get a pointer to the start of tcp data in buf
// Returns 0 if there is no data
// You must call init_len_info once before calling this function
uint16_t _getTCPDataOffset (struct ip_network *self)
{
	if (self->info_data_len > 0){
		return((uint16_t)TCP_SRC_PORT_H_P + self->info_hdr_len);
	}
	else{
		return(0);
	}
}



//*****************************************************************************
// do some basic length calculations and store the result in static varibales
void _parseLenInfo (struct ip_network *self)
{
	uint8_t *buf = self->buf; 
	self->info_data_len = (buf[IP_TOTLEN_H_P] << 8) | (buf[IP_TOTLEN_L_P] & 0xff);
	self->info_data_len -= IP_HEADER_LEN;
	self->info_hdr_len = (buf[TCP_HEADER_LEN_P] >> 4) * 4; // generate len in bytes;
	self->info_data_len -= self->info_hdr_len;
	if (self->info_data_len <= 0){
		self->info_data_len = 0;
	}
}



//*****************************************************************************
// fill in tcp data at position pos. pos=0 means start of
// tcp data. Returns the position at which the string after
// this string could be filled.
uint16_t _initTCPDataP (struct ip_network *self, uint16_t pos, const prog_char *progmem_s)
{
	char c;
	// fill in tcp data at position pos
	// with no options the data starts after the checksum + 2 more bytes (urgent ptr)
	while ((c = pgm_read_byte(progmem_s++))) 

	{
		self->buf[TCP_CHECKSUM_L_P + 3 + pos] = c;
		pos++;
	}
	return(pos);
}



//*****************************************************************************
// fill in tcp data at position pos. pos=0 means start of
// tcp data. Returns the position at which the string after
// this string could be filled.
uint16_t _initTCPData (struct ip_network *self, uint16_t pos, const char *s)
{
   // fill in tcp data at position pos
   // with no options the data starts after the checksum + 2 more bytes (urgent ptr)
   while (*s > 0) 

   {
      self->buf[TCP_CHECKSUM_L_P + 3 + pos] = *s;
      pos++;
      s++;
   }
   return(pos);
}



//*****************************************************************************
// Make just an ack packet with no tcp data inside
// This will modify the eth/ip/tcp header 
void _initTCPAck (struct ip_network *self)
{
	uint16_t j;
	uint8_t *buf = self->buf; 

	_initEthernet(self, buf + ETH_SRC_MAC);

	// fill the header:
	buf[TCP_FLAG_P] = TCP_FLAG_ACK_V;
	if (self->info_data_len == 0){
		// if there is no data then we must still acknoledge one packet
		_initTCPHeader(self, 1, 0, 1); // no options
	} else {
		_initTCPHeader(self, self->info_data_len, 0, 1); // no options
	}

	// total length field in the IP header must be set:
	// 20 bytes IP + 20 bytes tcp (when no options) 
	_initIP(self, buf + IP_SRC_P, IP_HEADER_LEN + TCP_HEADER_LEN_PLAIN);

	// calculate the checksum, len=8 (start from ip.src) + TCP_HEADER_LEN_PLAIN + data len
	j = Checksum(&buf[IP_SRC_P], 8 + TCP_HEADER_LEN_PLAIN, 2);
	buf[TCP_CHECKSUM_H_P] = j >> 8;
	buf[TCP_CHECKSUM_L_P] = j & 0xff;
	
	self->len = IP_HEADER_LEN + TCP_HEADER_LEN_PLAIN + ETH_HEADER_LEN;
}



//*****************************************************************************
// you must have called init_len_info at some time before calling this function
// dlen is the amount of tcp data (http data) we send in this packet
// You can use this function only immediately after make_tcp_ack_from_any
// This is because this function will NOT modify the eth/ip/tcp header except for
// length and checksum
void _initTCPAckWithData(struct ip_network *self, uint16_t dlen)
{
	uint16_t j;
	uint8_t *buf = self->buf; 
	// fill the header:
	// This code requires that we send only one data packet
	// because we keep no state information. We must therefore set
	// the fin here:
	buf[TCP_FLAG_P] = TCP_FLAG_ACK_V | TCP_FLAG_PUSH_V | TCP_FLAG_FIN_V;

	// total length field in the IP header must be set:
	// 20 bytes IP + 20 bytes tcp (when no options) + len of data
	j = IP_HEADER_LEN + TCP_HEADER_LEN_PLAIN + dlen;
	buf[IP_TOTLEN_H_P] = j >> 8;
	buf[IP_TOTLEN_L_P] = j & 0xff;
	
	_updateIPChecksum(self);
	
	// zero the checksum
	buf[TCP_CHECKSUM_H_P] = 0;
	buf[TCP_CHECKSUM_L_P] = 0;
	// calculate the checksum, len=8 (start from ip.src) + TCP_HEADER_LEN_PLAIN + data len
	j = Checksum(&buf[IP_SRC_P], 8 + TCP_HEADER_LEN_PLAIN + dlen, 2);
	buf[TCP_CHECKSUM_H_P] = j >> 8;
	buf[TCP_CHECKSUM_L_P] = j & 0xff;
	
	self->len = IP_HEADER_LEN + TCP_HEADER_LEN_PLAIN + dlen + ETH_HEADER_LEN;
}


//*****************************************************************************
uint16_t _tcpGetDataLength (struct ip_network *self)
{
	int dlength, hlength;
	uint8_t *buf = self->buf; 
	
	dlength = (buf[IP_TOTLEN_H_P] << 8) | buf[IP_TOTLEN_L_P];
	dlength -= IP_HEADER_LEN;
	hlength = (buf[TCP_HEADER_LEN_P] >> 4) * 4; // generate len in bytes;
	dlength -= hlength;
	if (dlength <= 0){
		dlength=0;
	}
	return ((uint16_t)dlength);
}


void ip_process_packets(struct ip_network *self){
	if(!self->buf) return; 
	
	while (1)
	{
		self->len = self->eth->read(self->eth, self->buf, self->max_buffer_size); 
		//enc28j60_PacketReceive(self->max_buffer_size, self->buf);
		
		if (self->len == 0) return;

		//uart_printf(PSTR("Plen: %d\n"), self->len); 
		
		// arp is broadcast if unknown but a host may also verify the mac address by sending it to a unicast address.
		if (_isArpPacketAddressedToUs(self))
		{
			/*uart_printf(PSTR("ARPSRCIP: %d.%d.%d.%d\n"), 
			buf[ETH_ARP_SRC_IP_P], buf[ETH_ARP_SRC_IP_P+1], 
			buf[ETH_ARP_SRC_IP_P+2], buf[ETH_ARP_SRC_IP_P+3]);
			uart_printf(PSTR("ARPSRCMAC: ")); 
			for(int c = 0; c < 6; c++) uart_printf(PSTR("%02x "), buf[ETH_ARP_SRC_MAC_P+c]); 
			uart_printf(PSTR("\n")); 
			uart_printf(PSTR("ARPDSTIP: ")); 
			for(int c = 0; c < 4; c++) uart_printf(PSTR("%d "), buf[ETH_ARP_DST_IP_P+c]); 
			uart_printf(PSTR("\n")); 
			uart_printf(PSTR("ARPDSTMAC: ")); 
			for(int c = 0; c < 6; c++) uart_printf(PSTR("%d "), buf[ETH_ARP_DST_MAC_P+c]); 
			uart_printf(PSTR("\n")); */
			_initARPResponse(self);
			ip_send(self); 
			continue;
		}
		
		// check if the ip packet is for us:
		if (_isIPPacketAddressedToUs(self) == 0)
		{
			
			/*uart_printf(PSTR("IPv: %d\n"), buf[IP_HEADER_LEN_VER_P]); 
			uart_printf(PSTR("FROM: %d.%d.%d.%d\n"), 
				buf[IP_SRC_P], buf[IP_SRC_P+1], 
				buf[IP_SRC_P+2], buf[IP_SRC_P+3]); 
			uart_printf(PSTR("TO: %d.%d.%d.%d\n"), 
				buf[IP_DST_P], buf[IP_DST_P+1], 
				buf[IP_DST_P+2], buf[IP_DST_P+3]); 
			uart_printf(PSTR("LEN (45): %d\n"), buf[IP_HEADER_LEN_VER_P]); */
			continue;
		}
		
		//uart_printf(PSTR("GOT PACKET!\n")); 
		
		// ping
		if((self->buf[IP_PROTO_P] == IP_PROTO_ICMP_V) && 
			(self->buf[ICMP_TYPE_P] == ICMP_TYPE_ECHOREQUEST_V))
		{
			 _initICMP(self);
			 ip_send(self); 
			 continue;
		}

		// tcp port www start, compare only the lower byte
		/*if ((buf[IP_PROTO_P] == IP_PROTO_TCP_V) && (buf[TCP_DST_PORT_H_P] == 0) && (buf[TCP_DST_PORT_L_P] == mywwwport))
		{
			 if (buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V)
			 {
					Make_tcp_synack_from_syn(buf); // make_tcp_synack_from_syn does already send the syn,ack
					continue;
			 }
			 if (buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V)
			 {
					Init_len_info(buf); // init some data structures
					dat_p = Get_tcp_data_pointer();
					if (dat_p == 0)
					{ 
						 // we can possibly have no data, just ack:
						 if (buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V)
						 {
								Make_tcp_ack_from_any(buf);
						 }
						 continue;
					}
			 }
			 send = false;
			 if (strncmp("GET ",(char *) & (buf[dat_p]), 4) != 0)
			 {
					// head, post and other methods for possible status codes see:
					// http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
					plen = Fill_tcp_data_p(buf, 0, PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 OK</h1>"));
					send = true;
			 }

			if (!send)
			 {
					char *p;
					p = (char*) &(buf[dat_p+4]);  // start of incomming payload
					switch (GetCommand(p))
					{
						 case 0:
								break;
						 case 1:
								countUp = false;
								break;
						 case 2:
								countUp = true;
								break;
					}
					plen = PrintWebpage(buf);
					send = true;
			 }

			 Make_tcp_ack_from_any(buf); // send ack for http get
			 Make_tcp_ack_with_data(buf, plen); // send data       
		}*/
	}
}
/* end of ip_arp_udp.c */
//*****************************************************************************
// The Ip checksum is calculated over the ip header only starting
// with the header length field and a total length of 20 bytes
// unitl ip.dst
// You must set the IP checksum field to zero before you start
// the calculation.
// len for ip is 20.
//
// For UDP/TCP we do not make up the required pseudo header. Instead we 
// use the ip.src and ip.dst fields of the real packet:
// The udp checksum calculation starts with the ip.src field
// Ip.src=4bytes,Ip.dst=4 bytes,Udp header=8bytes + data length=16+len
// In other words the len here is 8 + length over which you actually
// want to calculate the checksum.
// You must set the checksum field to zero before you start
// the calculation.
// len for udp is: 8 + 8 + data length
// len for tcp is: 4+4 + 20 + option len + data length
//
// For more information on how this algorithm works see:
// http://www.netfor2.com/checksum.html
// http://www.msc.uky.edu/ken/cs471/notes/chap3.htm
// The RFC has also a C code example: http://www.faqs.org/rfcs/rfc1071.html
static uint16_t Checksum(uint8_t *buf, uint16_t len, uint8_t type)

{
   // type 0=ip 
   //      1=udp
   //      2=tcp
   uint32_t sum = 0;

   if(type == 1)

   {
      sum += IP_PROTO_UDP_V; // protocol udp
      // the length here is the length of udp (data+header len)
      // =length given to this function - (IP.scr+IP.dst length)
      sum += (len - 8); // = real tcp len
   }
   if(type == 2)

   {
      sum += IP_PROTO_TCP_V; 
      // the length here is the length of tcp (data+header len)
      // =length given to this function - (IP.scr+IP.dst length)
      sum += (len - 8); // = real tcp len
   }
   // build the sum of 16bit words
   while(len > 1)

   {
      sum += 0xFFFF & ((*buf << 8) | *(buf+1));
      buf += 2;
      len -= 2;
   }
   // if there is a byte left then add it (padded with zero)
   if (len > 0)

   {
      sum += ((0xFF & *buf) << 8);
   }
   // now calculate the sum over the bytes in the sum
   // until the result is only 16bit long
   while (sum >> 16)

   {
      sum = (sum & 0xFFFF) + (sum >> 16);
   }
   // build 1's complement:
   return ((uint16_t)sum ^ 0xFFFF);
}

