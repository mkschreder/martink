#ifndef IP_ARP_UDP_TCP_H
#define IP_ARP_UDP_TCP_H


#ifdef __cplusplus
extern "C" {
#endif

/** 
 * 	Maintainer: Martin K. Schröder 
 * 
 *  Thanks for the code from ip_arp_udp_tcp.c!
 * 
 *  Date: 2014
 * 
 * 	info@fortmax.se
 */

struct ip_network {
	eth_mac_t macaddr;
	ip_addr_t ipaddr;
	ip_addr_t netmask; 
	int16_t info_hdr_len;
	int16_t info_data_len;
	uint32_t seqnum; // my initial tcp sequence number
	uint16_t ip_identifier;
	uint8_t *buf; 
	uint16_t max_buffer_size; 
	uint16_t len; 
	uint16_t port;

	struct packet_interface *eth;
};

typedef char PROGMEM prog_char; 

void ip_init(struct ip_network *self, struct packet_interface *eth, ip_addr_t ip, uint8_t *buffer, uint16_t size); 
void ip_process_packets(struct ip_network *self); 
void ip_send_udp(struct ip_network *self, const uint8_t *data, uint8_t datalen, 
	uint16_t sport, eth_mac_t dmac, ip_addr_t dip, uint16_t dport); 
//void ip_send(void); 

#ifdef __cplusplus
}
#endif

#endif /* IP_ARP_UDP_TCP_H */

