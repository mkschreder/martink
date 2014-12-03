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
 
#include <avr/pgmspace.h>

typedef uint8_t ip_addr_t[4]; 
typedef uint8_t ip_mac_t[6]; 

typedef char PROGMEM prog_char; 

void ip_init(ip_mac_t mac, ip_addr_t ip, uint8_t *buffer, uint16_t size); 
void ip_process_packets(void); 
void ip_send_udp(const uint8_t *data, uint8_t datalen, 
	uint16_t sport, ip_mac_t dmac, ip_addr_t dip, uint16_t dport); 
void ip_send(void); 

#ifdef __cplusplus
}
#endif

#endif /* IP_ARP_UDP_TCP_H */

