﻿/*!****************************************************************************
 * @file		pingServer.c
 * @author		d_el - Storozhenko Roman
 * @version		V1.0
 * @date		22.09.2017
 * @copyright	GNU Lesser General Public License v3
 * @brief		Ping service
 */

/*!****************************************************************************
 * Include
 */
#include "pingService.h"
#include "lwip/opt.h"
#include "lwipopts.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/prot/ip.h"
#include "assert.h"
#include "printp.h"

/*!****************************************************************************
 * MEMORY
 */
static struct raw_pcb 	*ping_raw_pcb;

/*!****************************************************************************
 * @brief	Ping using the raw ip
 */
static u8_t pingRecv(void *arg, struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *addr){
	struct icmp_echo_hdr *iecho;
	assert(p != NULL);

	if(pbuf_header(p, -PBUF_IP_HLEN) == 0){
		iecho = (struct icmp_echo_hdr *) p->payload;
		ICMPH_TYPE_SET(iecho, ICMP_ER);
		ICMPH_CODE_SET(iecho, 0);
		iecho->chksum = 0;
		#if HECKSUM_CHECK_ICMP
		inet_chksum(iecho, p->len);
		#endif
		raw_sendto(ping_raw_pcb, p, addr);
	}

	pbuf_free(p);
	return 1; /* Eat the packet. See description raw_recv */
}

/*!****************************************************************************
 * @brief	Initialization receive ICMP from any IP
 */
void ping_init(void){
	ping_raw_pcb = raw_new(IP_PROTO_ICMP);
	assert(ping_raw_pcb != NULL);
	raw_recv(ping_raw_pcb, pingRecv, NULL);
	raw_bind(ping_raw_pcb, IP_ADDR_ANY);
}

/*************** LGPL ************** END OF FILE *********** D_EL ************/
