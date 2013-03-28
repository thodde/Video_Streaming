////////////////////////////////////////////////////////////
/*
	File Name:		AddressUtility.c
	Author:			Trevor Hodde
	Note:			This AddressUtility.c file includes 
					Handle Socket Address Functions.
*/
////////////////////////////////////////////////////////////

///////////////HEADER FILES///////////////
#include <arpa/inet.h>
#include "Client.h"

///////////////FUNCTIONS///////////////
/*Print Socket Address Function
  Variable Definition:
  -- address: socket address structure
  -- stream: file stream(stdin, stdout, stderr, or file objective...)
  Return Value: NULL
*/
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
		//The type of address is ipv6
		case AF_INET6:
			numeric_address = &((struct sockaddr_in6*)address)->sin6_addr;
			port = ntohs(((struct sockaddr_in6*)address)->sin6_port);
			break;
		//Invalid address
		default:
			fputs("[unknown type]", stream);
			return;
	}
	
	//Convert binary to printable address
	if (inet_ntop(address->sa_family, numeric_address, address_buffer, sizeof(address_buffer)) == NULL){
		//Unable to convert
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
