/////////////////////////////////////////////////////////////
/*
	File Name:		ClientUtility.c
	Author:			Trevor Hodde
	Note:			This ClientUtility.c file includes 
					Initialize Client, Setup TCP Socket,
					and Setup UDP Socket Functions.
*/
/////////////////////////////////////////////////////////////

///////////////HEADER FILES///////////////
#include "Client.h"

///////////////FUNCTIONS///////////////
/*Initialize Client Function
  Variable Definition:
  -- port: client rtp port number
  Return value: NULL
*/
void initClient(int port){
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

/*Setup Client Socket Function
  Variable Definition:
  -- host: socket host name/ip address
  -- service: socket service name/port number
  Return value: socket number
*/
int setupClientTCPSocket(const char *host, const char *service){
	struct addrinfo		address_criteria;		//criteria for address match
	struct addrinfo		*server_address;		//holder for returned list of server addresses
	struct addrinfo		*address;				//pointer to addresses node
	int					client_socket = -1;		//socket descriptor for client
	int					return_value;			//return value
	
	//Tell the system what kind(s) of address info we want
	memset(&address_criteria, 0, sizeof(address_criteria));	//zero the address_criteria
	address_criteria.ai_family = AF_UNSPEC;					//any address family
	address_criteria.ai_socktype = SOCK_STREAM;				//only streaming sockets
	address_criteria.ai_protocol = IPPROTO_TCP;				//only TCP protocol
	
	//Get address(es)
	return_value = getaddrinfo(host, service, &address_criteria, &server_address);
	//Success returns zero
	if (return_value != 0){
		perror("getaddrinfo() failed");
	}
	
	//Create socket for connecting the server
	for (address = server_address; address != NULL; address = address->ai_next){
		//Create a reliable, stream socket using TCP
		client_socket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
		//Socket creation failed; try next address
		if (client_socket < 0){
			continue;
		}
		//Establish the connection to the server
		if (connect(client_socket, address->ai_addr, address->ai_addrlen) == 0){
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

/*Setup Server UDP Socket Function
  Variable Definition:
  -- service: socket service name/port number
  Return value: socket number
*/
int setupClientUDPSocket(const char *service){
	struct addrinfo 		address_criteria;		//criteria for address match
	struct addrinfo 		*server_address;		//list of server addresses
	struct addrinfo 		*address;				//pointer to addresses node
	struct sockaddr_storage local_address;			//print local address
	socklen_t 				address_size;			//address size
	int						client_socket = -1;		//socket descriptor for client
	int 					return_value;			//return value
	
	//Tell the system what kind(s) of address info we want
	memset(&address_criteria, 0, sizeof(address_criteria));	//zero the address_criteria
	address_criteria.ai_family = AF_UNSPEC;					//any address family
	address_criteria.ai_flags = AI_PASSIVE;					//accept on any address/port
	address_criteria.ai_socktype = SOCK_DGRAM;				//only datagram sockets
	address_criteria.ai_protocol = IPPROTO_UDP;				//only udp protocol
	
	//Get address(es)
	return_value = getaddrinfo(NULL, service, &address_criteria, &server_address);
	//Success returns zero
	if (return_value != 0){
		perror("getaddrinfo() failed!");
	}
	
	//Create socket for incoming connections
	for (address = server_address; address != NULL; address = address->ai_next){
		//Initialize the client socket
		client_socket = -1;
		//Create socket for incoming connections
		client_socket = socket(server_address->ai_family, server_address->ai_socktype, server_address->ai_protocol);
		//if socket creation failed, try next address in the list
		if (client_socket < 0){
			continue;
		}
		
		//Bind to the client local address and set socket to the list
		if (bind(client_socket, server_address->ai_addr, server_address->ai_addrlen) == 0){
			//Get address size
			address_size = sizeof(local_address);
			//Get socket name
			if (getsockname(client_socket, (struct sockaddr*)&local_address, &address_size) < 0){
				perror("getsockname() failed!");
			}
			//Output local address and port of socket(listening address and port)
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
