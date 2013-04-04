/**
 * File Name: SendUtility.c
 * Author:    Trevor Hodde
 */

#include "Server.h"

void sendOK(char *url, const char *method, u_int32 cseq, int client_socket) {
	//Handle the SETUP method
	if (methodIsSetup(method)) {
		char *entity_body = (char*)malloc(sizeof(char) * (HALFBUF_SIZE + 1));
		char	*transport = (char*)malloc(sizeof(char) * (STRING_SIZE + 1));

		//Set the transport information
		sprintf(transport, "%s; %s; client_port=%d; server_port=%d; mode=PLAY", protocol_type,
			protocol_method,
			client_rtp_port,
			server_rtp_port);

		//Set the entity_body
		sprintf(entity_body, "frame_number=%u", getVideoInfo(url));
		//SETUP method Response Message
		constructResponseMessage(client_socket,
					NULL,
					200,
					"OK",
					cseq,
					transport,
					convertTimeFormat(getTimeInGMTFormat(url, 2), 1),
					strlen(entity_body) + 1,
					"text/plain",
					entity_body);
		//Set the status
		status = READY;
	}
	else if (methodIsPlay(method)) {
		//PLAY method Response Message
		constructResponseMessage(client_socket,
					NULL,
					200,
					"OK",
					cseq,
					NULL,
					convertTimeFormat(getTimeInGMTFormat(url, 2), 1),
					0,
					NULL,
					NULL);
		//Set the status
		status = PLAYING;
		//Start RTP progress
		startRTPProgress();
	}
	else if (methodIsTeardown(method)) {
		constructResponseMessage(client_socket,
					NULL,
					200,
					"OK",
					cseq,
					NULL,
					convertTimeFormat(getTimeInGMTFormat(url, 2), 1),
					0,
					NULL,
					NULL);

		//Close stream for video file
		closeVideoStream();
		//Set the status
		status = INIT;
	}

	return;
}

void sendNotModified(char *url, u_int32 cseq, int client_socket) {
	//Send Response to client
	constructResponseMessage(client_socket,
				NULL,
				304,
				"Not Modified",
				cseq,
				NULL,
				convertTimeFormat(getTimeInGMTFormat(url, 2), 1),
				0,
				NULL,
				NULL);

	return;
}
