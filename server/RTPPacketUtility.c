/////////////////////////////////////////////////////////////////
/*
	File Name:		RTPPacketUtility.c
	Author:			Trevor Hodde
	Note:			This RTPPacketUtility.c file includes 
					Handle RTP Packets Function.
*/
/////////////////////////////////////////////////////////////////

///////////////HEADER FILES///////////////
#include <time.h>
#include "Server.h"

///////////////GLOBAL VARIABLES///////////////
static struct itimerval		rtp_timer;			//rtp packet timer
static char			*video_file_name;			//video file name
static long			next_frame_start = 0;		//next frame start number
static u_int16		frame_number = 0;			//sequence number
static u_int32		frame_total = 0;			//total frame number
static u_int32		ssrc = 0;					//rtp ssrc field

///////////////FUNCTIONS///////////////
/*Get Video Information Function
  Variable Definition:
  -- url: the request url except domain name and port number
  Return Value: NULL
*/
u_int32 getVideoInfo(char *url){
	FILE		*video_channel;				//video file stream
	u_int8		frame_length[NUMBER_SIZE];	//frame length string
	u_int32		count = 0;					//counter for frame number
	size_t		length = 0;					//frame length

	//Initialize frame_length buffer
	memset(frame_length, 0, NUMBER_SIZE);
	//Open video stream channel
	video_channel = fopen(url, "r");
	if (video_channel == NULL){
		perror("fopen() failed");
	}
	//First get the frame length
	while (fread(frame_length, NUMBER_SIZE, ONE_SIZE, video_channel) == 1){
		//Set the next frame start postion
		next_frame_start += NUMBER_SIZE;
		//Convert the string to size_t
		length = atoi((char*)frame_length);
		//Set the next frame start position
		next_frame_start += length;
		//Increase the frame_number
		count++;
		//Seek the video file
		fseek(video_channel, next_frame_start, 0);
	}
	//Close video file stream
	fclose(video_channel);

	//Get the request video file name
	video_file_name = url;
	//Set the ssrc number
	ssrc = getRandomNumber(BASIC_NUMBER, MOD_NUMBER);
	//Set the frame total
	frame_total = count;
	//Initialize the next frame start number
	next_frame_start = 0;

	return count;
}

/*Start RTP Progress Function
  Variable Definition:
  -- NULL
  Return Value: NULL
*/
void startRTPProgress(){
	//Set the timer
	setTimer(rtp_timer, ITIMER_REAL, 0, FRAME_PERIOD, 0, FRAME_PERIOD);

	return;
}

/*Stop RTP Progress Function
  Variable Definition:
  -- NULL
  Return Value: NULL
*/
void stopRTPProgress(){
	//Clear the timer
	setTimer(rtp_timer, ITIMER_REAL, 0, 0, 0, 0);

	return;
}

/*Close Video Stream Function
  Variable Definition:
  -- NULL
  Return Value: NULL
*/
void closeVideoStream(){
	//Test the server status
	if (status == PLAYING){
		stopRTPProgress();
	}
	//Initialize RTP packet variables
	video_file_name = NULL;
	next_frame_start = 0;
	frame_number = 0;
	frame_total = 0;
	ssrc = 0;
	//Initialize the server variables
	initServer(SERVER_RTP_PORT);

	return;
}

/*Set RTP Packet Header Function
  Variable Definition:
  -- rtp: _rtp_header structure node
  Return value: NULL
*/
void setRTPPacketHeader(RTP_HEADER *rtp){
	char	*type = getFileType(video_file_name);	//video file type

	//Set the version, padding, extension, and csrc count field
	rtp->vpxcc = RTP_VERSION | PADDING | EXTENSION | CSRC_COUNT;
	//Set the marker and payload type field
	if (strcmp(type, "Mjpeg") == 0){
		rtp->mpt = MARKER | PAYLOAD_TYPE;
	}
	else{
		rtp->mpt = MARKER | 0xff;
	}
	//Set the sequence field
	rtp->seq_number = frame_number;
	rtp->seq_number = htons(rtp->seq_number);
	//Set the timestamp field
	rtp->timestamp = time(NULL);
	rtp->timestamp = htonl(rtp->timestamp);
	//Set the SSRC field
	if (ssrc == 0){
		//Get the ssrc
		ssrc = getRandomNumber(BASIC_NUMBER, MOD_NUMBER);
	}
	rtp->ssrc = ssrc;
	rtp->ssrc = htonl(rtp->ssrc);

	return;
}

/*Construct RTP Packet Function
  Variable Definition:
  -- length: rtp packet payload length pointer
  Return value: rtp packet (including head and payload)
*/
u_int8 *constructRTPPacket(size_t *length){
	FILE		*video_channel;					//video file stream
	u_int8		frame_length[NUMBER_SIZE];		//frame length string
	size_t		count;							//counter

	//Open video file stream
	video_channel = fopen(video_file_name, "r");
	//Seek the next frame start position
	fseek(video_channel, next_frame_start, 0);
	//First step: read the length of frame
	if (fread(frame_length, NUMBER_SIZE, ONE_SIZE, video_channel) != 1){
		perror("fread() failed");
	}
	//Set the next frame start position
	next_frame_start += NUMBER_SIZE;
	//Convert the string to size_t
	*length = atoi((char*)frame_length);
	//Set the next frame start position
	next_frame_start += (*length);

	u_int8		*frame_content = (u_int8*)malloc(sizeof(u_int8) * (*length));				//frame content
	u_int8		*buffer = (u_int8*)malloc(sizeof(u_int8) * (RTP_HEAD_SIZE + (*length)));	//rtp packet buffer
	RTP_HEADER	*rtp_head = (RTP_HEADER*)buffer;											//_rtp_header structure node

	//Set the rtp packet header
	setRTPPacketHeader(rtp_head);
	//Second step: read the frame content according to the frame length
	if ((fread(frame_content, *length, ONE_SIZE, video_channel) != 1)){
		perror("fread() failed: unable to read frame content!");
	}
	//Add the frame content into the buffer
	for (count = 0; count < (*length); count++){
		buffer[count + RTP_HEAD_SIZE] = frame_content[count];
	}
	//Close video file stream
	fclose(video_channel);

	return buffer;
}

/*Handle for signal Function
  Variable Definition:
  -- ignored: signal type
  Return value: NULL
*/
void catchAlarm(int ignored){
	UDP_SOCKET_INFO		*udp_server_socket;			//_udp_socket_info structure node
	u_int8				*buffer;					//rtp packet buffer
	size_t				length = 0;					//frame length
	ssize_t				number_bytes;				//size of sending message

	//Test the frame number
	if (frame_total <= frame_number){
		//Stop the timer
		stopRTPProgress();
		//Initialize the frame number and next frame start value
		frame_number = 0;
		next_frame_start = 0;
		return;
	}
	//Construct RTP packet
	buffer = constructRTPPacket(&length);
	//Create socket for sending rtp packets
	udp_server_socket = setupServerUDPSocket(rtp_address, itoa(client_rtp_port));
	if (udp_server_socket->socket < 0){
		perror("setupServerUDPSocket() failed");
	}
	//Output the socket address and port
	fputs("Sending RTP packet to client ", stdout);
	printSocketAddress((struct sockaddr*)udp_server_socket->address->ai_addr, stdout, true);
	//Output frame number
	printf(", frame number: #%u\n", frame_number);

	//Send the rtp packet
	number_bytes = sendto(	udp_server_socket->socket,
							buffer,
							RTP_HEAD_SIZE + length,
							MSG_NOSIGNAL,
							udp_server_socket->address->ai_addr,
							udp_server_socket->address->ai_addrlen);
	//Test the sending is successful
	if (number_bytes < 0){
		perror("sendto() failed");
	}
	else if (number_bytes != (RTP_HEAD_SIZE + length)){
		perror("sendto() error: send unexpected number of bytes!");
	}
	//Close server socket
	close(udp_server_socket->socket);
	//Free the _udp_socket_info structure
	free(udp_server_socket);
	//Increase the sequence number
	frame_number++;

	return;
}
