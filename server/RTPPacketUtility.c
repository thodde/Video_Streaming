/**
 * File Name: RTPPacketUtility.c
 * Author:    Trevor Hodde
 * This file creates and sends the RTP packets.
 * Some of the RTP Packet code borrowed from:
 * docs.gstreamer.com
 */

#include <time.h>
#include "Server.h"

static struct itimerval	rtp_timer;	//rtp packet timer
static char    *video_file_name;	//video file name
static long    next_frame_start = 0;	//next frame start number
static u_int16 frame_number = 0;	//sequence number
static u_int32 frame_total = 0;	        //total frame number
static u_int32 ssrc = 0;		//rtp ssrc field

//This makes sure the file type is valid
char *getFileType(char *url) {
	char *extension = NULL;
	
	//Get the file extension type using the '.' character
	if ((extension = strrchr(url, '.')) != NULL) {
		//Jump the '.' character
		return (extension + 1);
	}
	return "";
}

u_int32 getVideoInfo(char *url) {
	FILE	*video_channel;	
	u_int8	frame_length[NUMBER_SIZE];
	u_int32	count = 0;	
	size_t	length = 0;

	//Initialize frame_length buffer
	memset(frame_length, 0, NUMBER_SIZE);
	//Open video stream channel
	video_channel = fopen(url, "r");

	if (video_channel == NULL) {
		perror("fopen() failed");
	}

	//First get the frame length
	while (fread(frame_length, NUMBER_SIZE, ONE_SIZE, video_channel) == 1) {
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
	ssrc = getRandomNumber(BASIC_NUMBER, MOD_NUMBER);
	frame_total = count;

	//Initialize the next frame start number
	next_frame_start = 0;

	return count;
}

void startRTPProgress() {
	setTimer(rtp_timer, ITIMER_REAL, 0, FRAME_PERIOD, 0, FRAME_PERIOD);
	return;
}

void stopRTPProgress(){
	setTimer(rtp_timer, ITIMER_REAL, 0, 0, 0, 0);
	return;
}

void closeVideoStream(){
	//Test the server status
	if (status == PLAYING) {
		stopRTPProgress();
	}

	//Initialize RTP packet variables
	video_file_name = NULL;
	next_frame_start = 0;
	frame_number = 0;
	frame_total = 0;
	ssrc = 0;
	initServer(SERVER_RTP_PORT);

	return;
}

void setRTPPacketHeader(RTP_HEADER *rtp){
	char	*type = getFileType(video_file_name);	

	//Set the version, padding, extension, and csrc count field
	rtp->vpxcc = RTP_VERSION | PADDING | EXTENSION | CSRC_COUNT;

	//Set the marker and payload type field
	if (strcmp(type, "Mjpeg") == 0) {
		rtp->mpt = MARKER | PAYLOAD_TYPE;
	}
	else {
		rtp->mpt = MARKER | 0xff;
	}

	//Set the sequence field
	rtp->seq_number = frame_number;
	rtp->seq_number = htons(rtp->seq_number);
	//Set the timestamp field
	rtp->timestamp = time(NULL);
	rtp->timestamp = htonl(rtp->timestamp);

	//Set the SSRC field
	if (ssrc == 0) {
		//Get the ssrc
		ssrc = getRandomNumber(BASIC_NUMBER, MOD_NUMBER);
	}
	rtp->ssrc = ssrc;
	rtp->ssrc = htonl(rtp->ssrc);

	return;
}

//This is where the RTP Packet is built
u_int8 *constructRTPPacket(size_t *length) {
	FILE	*video_channel;	
	u_int8	frame_length[NUMBER_SIZE];
	size_t	count;

	//Open video file stream
	video_channel = fopen(video_file_name, "r");
	//Seek the next frame start position
	fseek(video_channel, next_frame_start, 0);

	//First step: read the length of frame
	if (fread(frame_length, NUMBER_SIZE, ONE_SIZE, video_channel) != 1) {
		perror("fread() failed");
	}

	//Set the next frame start position
	next_frame_start += NUMBER_SIZE;
	//Convert the string to size_t
	*length = atoi((char*)frame_length);
	//Set the next frame start position
	next_frame_start += (*length);

	u_int8	*frame_content = (u_int8*)malloc(sizeof(u_int8) * (*length));
	u_int8	*buffer = (u_int8*)malloc(sizeof(u_int8) * (RTP_HEAD_SIZE + (*length)));
	RTP_HEADER *rtp_head = (RTP_HEADER*)buffer;

	//Set the rtp packet header
	setRTPPacketHeader(rtp_head);

	//Second step: read the frame content according to the frame length
	if ((fread(frame_content, *length, ONE_SIZE, video_channel) != 1)) {
		perror("fread() failed: unable to read frame content!");
	}

	//Add the frame content into the buffer
	for (count = 0; count < (*length); count++) {
		buffer[count + RTP_HEAD_SIZE] = frame_content[count];
	}

	//Close video file stream
	fclose(video_channel);

	return buffer;
}

void catchAlarm(int ignored) {
	UDP_SOCKET_INFO	*udp_server_socket;
	u_int8 *buffer;	
	size_t	length = 0;
	ssize_t	number_bytes;

	//Test the frame number
	if (frame_total <= frame_number) {
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

	if (udp_server_socket->socket < 0) {
		perror("setupServerUDPSocket() failed");
	}
	//display the socket address and port
	fputs("Sending RTP packet to client ", stdout);
	printSocketAddress((struct sockaddr*)udp_server_socket->address->ai_addr, stdout, true);
	//print frame number
	printf(", frame number: #%u\n", frame_number);

	//Send the rtp packet
	number_bytes = sendto(udp_server_socket->socket, buffer,
				RTP_HEAD_SIZE + length, MSG_NOSIGNAL,
				udp_server_socket->address->ai_addr,
				udp_server_socket->address->ai_addrlen);

	//Test the sending is successful
	if (number_bytes < 0) {
		perror("sendto() failed");
	}
	else if (number_bytes != (RTP_HEAD_SIZE + length)) {
		perror("sendto() error: trying to send unexpected number of bytes!");
	}

	//Close server socket
	close(udp_server_socket->socket);
	//Free the _udp_socket_info structure
	free(udp_server_socket);
	//Increase the sequence number
	frame_number++;

	return;
}
