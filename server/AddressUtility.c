/**
 * File Name: AddressUtility.c
 * Author:    Trevor Hodde
 * This is mostly borrowed from TCP/IP Sockets in C
 * By Donahoo and Calvert
 */

#include <arpa/inet.h>
#include "Server.h"

void printSocketAddress(const struct sockaddr *address, FILE *stream, bool get_address) {
	void 	*numeric_address;
	char 	address_buffer[INET6_ADDRSTRLEN];
	in_port_t 	port;
	
	//Test for address and stream
	if (address == NULL || stream == NULL) {
		return;
	}
	
	switch (address->sa_family) {
		case AF_INET:
			numeric_address = &((struct sockaddr_in*)address)->sin_addr;
			port = ntohs(((struct sockaddr_in*)address)->sin_port);
			break;
		//Invalid address
		default:
			fputs("[unknown type]", stream);
			return;
	}
	
	if (inet_ntop(address->sa_family, numeric_address, address_buffer, sizeof(address_buffer)) == NULL) {
		//Unable to convert
		fputs("[invalid address]", stream);
	}
	else {
		//Output the address
		fprintf(stream, "%s", address_buffer);
		if (port != 0) {
			//Output the port number
			fprintf(stream, "-%u", port);
		}
	}

	//Get the address
	if (get_address) {
		strcpy(rtp_address, address_buffer);
	} 

	return;
}
