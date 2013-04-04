/**
 * File Name: ClientUtility.c
 * Author:    Trevor Hodde
 * TCP Socket Code borrowed from:
 * 	TCP/IP Sockets in C By Donahoo and Calvert
 * UDP Socket Code borrowed from:
 * 	beej.us/guide/bgnet/output/html/multipage/clientserver.html
 */

#include "Client.h"

void initClient(int port) {
	//Initialize session id number
	session_id = 0;
	//Initialize cseq number
	cseq_number = 0;
	//Initialize client status
	status = INIT;
	//Initialize client rtp port & rtcp port number
	client_rtp_port = port;
	client_rtcp_port = client_rtp_port + ONE_SIZE;
	//Initialize server rtp port number
	server_rtp_port = 0;

	return;
}

int setupClientTCPSocket(const char *host, const char *service){
	struct addrinfo		address_criteria;
	struct addrinfo		*server_address;
	struct addrinfo		*address;
	int	client_socket = -1;
	int	return_value;
	
	//Tell the system what kind(s) of address info we want
	memset(&address_criteria, 0, sizeof(address_criteria));	
	address_criteria.ai_family = AF_UNSPEC;			
	address_criteria.ai_socktype = SOCK_STREAM;	
	address_criteria.ai_protocol = IPPROTO_TCP;
	
	//Get address
	return_value = getaddrinfo(host, service, &address_criteria, &server_address);
	//Success returns zero
	if (return_value != 0) {
		perror("getaddrinfo() failed");
	}
	
	//Create socket for connecting the server
	for (address = server_address; address != NULL; address = address->ai_next) {
		//Create a reliable, stream socket using TCP
		client_socket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
		//Socket creation failed; try next address
		if (client_socket < 0) {
			continue;
		}

		//Establish the connection to the server
		if (connect(client_socket, address->ai_addr, address->ai_addrlen) == 0) {
			//Socket connection succeeded; break and return socket
			break;
		}

		//Socket connection failed; try next address
		close(client_socket);
		client_socket = -1;
	}
	//Free addrinfo allocated in getaddrinfo()
	freeaddrinfo(server_address);
	
	return client_socket;
}

int setupClientUDPSocket(const char *service){
	struct addrinfo 	address_criteria;
	struct addrinfo 	*server_address;
	struct addrinfo 	*address;
	struct sockaddr_storage local_address;	
	socklen_t 	address_size;
	int	client_socket = -1;
	int 	return_value;	
	
	//Tell the system what kind of address want
	memset(&address_criteria, 0, sizeof(address_criteria));	
	address_criteria.ai_family = AF_UNSPEC;		
	address_criteria.ai_flags = AI_PASSIVE;	
	address_criteria.ai_socktype = SOCK_DGRAM;
	address_criteria.ai_protocol = IPPROTO_UDP;
	
	//Get address
	return_value = getaddrinfo(NULL, service, &address_criteria, &server_address);
	//Success returns zero
	if (return_value != 0) {
		perror("getaddrinfo() failed!");
	}
	
	//Create socket for incoming connections
	for (address = server_address; address != NULL; address = address->ai_next) {
		//Initialize the client socket
		client_socket = -1;
		//Create socket for incoming connections
		client_socket = socket(server_address->ai_family, server_address->ai_socktype, server_address->ai_protocol);
		//if socket creation failed, try next address in the list
		if (client_socket < 0) {
			continue;
		}
		
		//Bind to the client local address and set socket to the list
		if (bind(client_socket, server_address->ai_addr, server_address->ai_addrlen) == 0) {
			//Get address size
			address_size = sizeof(local_address);
			//Get socket name
			if (getsockname(client_socket, (struct sockaddr*)&local_address, &address_size) < 0) {
				perror("getsockname() failed!");
			}

			fputs("Binding to ", stdout);
			printSocketAddress((struct sockaddr*)&local_address, stdout);
			fputc('\n', stdout);
			//Bind and list successful
			break;
		}
		//Close and try again
		close(client_socket);
	}
	//Free address list allocated by getaddrinfo()
	freeaddrinfo(server_address);
	
	return client_socket;
}
