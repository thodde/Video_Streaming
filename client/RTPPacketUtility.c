/////////////////////////////////////////////////////////////////
/*
	File Name:		RTPPacketUtility.c
	Instructor:		Prof. Arthur Goldberg
	Author:			Trevor Hodde
	UID:			N14361265
	Department:		Computer Science
	Note:			This RTPPacketUtility.c file includes 
					Handle RTP Packets Function.
*/
/////////////////////////////////////////////////////////////////

///////////////HEADER FILES///////////////
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include "Client.h"

///////////////GLOBAL VARIABLES///////////////
static int			rtp_client = -1;		//udp client socket
static u_int16		frame_number = 0;		//sequence number
static u_int32		ssrc = 0;				//ssrc number

///////////////FUNCTIONS///////////////
/*Start RTP Progress Function
  Variable Definition:
  -- client_data: callback function widget
  Return Value: NULL
*/
void startRTPProgress(CLIENT_DATA *client_data){
	struct sigaction	handler;								//sigaction structure
	u_int32				rcvd_buffer_size = 50 * BUFFER_SIZE;	//received buffer size

	//Create socket for incoming connections
	rtp_client = setupClientUDPSocket(itoa(client_rtp_port));
	if (rtp_client < 0){
		showErrorCallback(client_data->window, "setupClientUDPSocket() failed: unable to connect!");
		return;
	}
	//Set the received buffer size
	setsockopt(rtp_client, SOL_SOCKET, SO_RCVBUF, &rcvd_buffer_size, sizeof(rcvd_buffer_size));

	//Set signal handler for SIGIOHandler
	handler.sa_handler = SIGIOHandler;
	//Create mask that mask all signals
	if (sigfillset(&handler.sa_mask) < 0){
		showErrorCallback(client_data->window, "sigfillset() failed!");
		return;
	}
	//No flags
	handler.sa_flags = 0;
	//Set the "SIGIO" signal
	if (sigaction(SIGIO, &handler, 0) < 0){
		showErrorCallback(client_data->window, "sigaction() failed for SIGIO!");
		return;
	}
	//We must own the socket to receive the SIGIO message
	if (fcntl(rtp_client, F_SETOWN, getpid()) < 0){
		showErrorCallback(client_data->window, "Unable to set process owner to us!");
		return;
	}
	//Arrage for nonblocking I/O and SIGIO delivery
	if (fcntl(rtp_client, F_SETFL, O_NONBLOCK | FASYNC) < 0){
		showErrorCallback(client_data->window, "Unable to put client sock into non-blocking/async mode!");
		return;
	}

	return;
}

/*Stop RTP Progress Function
  Variable Definition:
  -- status_signal: status value
  Return Value: NULL
*/
void stopRTPProgress(u_int32 status_signal){
	//Close udp client socket
	if (rtp_client != -1){
		close(rtp_client);
	}
	//Initialize udp client socket
	rtp_client = -1;
	switch (status_signal){
		case INIT:
			//Initialize frame number
			frame_number = 0;
			//Initialize ssrc
			ssrc = 0;
			break;
		default:
			break;
	}

	return;
}

/*Check RTP Header Packet Function
  Variable Definition:
  -- rtp: rtp packet buffer
  Return Value: if rtp header is correct, return true, else return false
*/
bool checkRTPHeader(const u_int8 *rtp){
	RTP_HEADER	*rtp_head = (RTP_HEADER*)rtp;		//_rtp_header structure node

	//Test the rtp version field
	if ((rtp_head->vpxcc & RTP_VERSION_MASK) != RTP_VERSION){
		fputs("RTP version is incorrect!\n", stdout);
		return false;
	}
	//Test the payload type field
	else if ((rtp_head->mpt & PAYLOAD_TYPE_MASK) != PAYLOAD_TYPE){
		fputs("RTP payload type is incorrect!\n", stdout);
		return false;
	}
	else if (ssrc == 0){
		//Set the ssrc value
		ssrc = ntohl(rtp_head->ssrc);
	}
	//Test the ssrc field
	else if (ntohl(rtp_head->ssrc) != ssrc){
		fputs("SSRC field value is incorrect!\n", stdout);
		return false;
	}
	//Set the frame number value
	frame_number = ntohs(rtp_head->seq_number);

	return true;
}

/*Handle SIGIO Signal Function
  Variable Definition:
  -- signal_type: signal type
  Return value: NULL
*/
void SIGIOHandler(int signal){
	u_int8		*buffer;				//datagram buffer
	ssize_t		number_bytes_rcvd;		//size of received message

	//Allocate memory for buffer
	buffer = (u_int8*)malloc(sizeof(u_int8) * FRAME_SIZE);
	//As long as there is input...
	do{
		struct sockaddr_storage	server_address;				//server address
		socklen_t				server_address_length;		//length of server address structure

		//Set length of server address structure
		server_address_length = sizeof(server_address);
		//Block until receive message from a server
		number_bytes_rcvd = recvfrom(	rtp_client,
										buffer,
										FRAME_SIZE,
										0,
										(struct sockaddr*)&server_address,
										&server_address_length);
		//Receive is failed
		if (number_bytes_rcvd < 0){
			//Only acceptable error: recvfrom() would have blocked
			if (errno != EWOULDBLOCK){
				perror("recvfrom() failed");
				return;
			}
		}
		//Receivei is successful
		else{
			//Now, client has received server message
			//Check the RTP packet header
			if (checkRTPHeader(buffer)){
				//Set the new image on the image widget
				setImage(buffer);
			}
			//Output the server address and port
			fputs("Receive RTP packet from ", stdout);
			printSocketAddress((struct sockaddr*)&server_address, stdout);
			//Output the frame number
			printf(", frame number: #%u\n", frame_number);
		}
	}while (number_bytes_rcvd >= 0);
	//Nothing left to receive
	return;
}
