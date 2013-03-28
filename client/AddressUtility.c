/*
	File Name:		AddressUtility.c
	Author:			Trevor Hodde
*/

#include <arpa/inet.h>
#include "Client.h"

void printSocketAddress(const struct sockaddr *address, FILE *stream){
	void 		*numeric_address;					//pointer to binary address
	char 		address_buffer[INET6_ADDRSTRLEN];	//address data
	in_port_t 	port;								//port number
	
	//Test for address and stream
	if (address == NULL || stream == NULL){
		return;
	}
	
	//Set pointer to address based on address family
	switch (address->sa_family){
		//The type of address is ipv4
		case AF_INET:
			numeric_address = &((struct sockaddr_in*)address)->sin_addr;
			port = ntohs(((struct sockaddr_in*)address)->sin_port);
			break;
		//Invalid address
		default:
			fputs("[unknown type]", stream);
			return;
	}
	
	//Convert binary to printable address
	if (inet_ntop(address->sa_family, numeric_address, address_buffer, sizeof(address_buffer)) == NULL){
		fputs("[invalid address]", stream);
	}
	else{
		//Output the address
		fprintf(stream, "%s", address_buffer);
		if (port != 0){
			//Output the port number
			fprintf(stream, "-%u", port);
		}
	}

	return;
}
