/*
	File Name:		SendUtility.c
	Author:			Trevor Hodde
*/

#include "Server.h"

void sendOK(char *url, const char *method, u_int32 cseq, int client_socket){
	//Handle the SETUP method
	if (methodIsSetup(method)){
		char	*entity_body = (char*)malloc(sizeof(char) * (HALFBUF_SIZE + 1));	//entity body
		char	*transport = (char*)malloc(sizeof(char) * (STRING_SIZE + 1));		//transport information

		//Set the transport information
		sprintf(transport, "%s; %s; client_port=%d; server_port=%d; mode=PLAY", protocol_type,
																				protocol_method,
																				client_rtp_port,
																				server_rtp_port);
		//Set the entity_body
		sprintf(entity_body, "frame_number=%u", getVideoInfo(url));
		//SETUP method Response Message
		constructResponseMessage(	client_socket,
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
	else if (methodIsPlay(method)){
		//PLAY method Response Message
		constructResponseMessage(	client_socket,
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
	else if (methodIsPause(method)){
		//PAUSE method Response Message
		constructResponseMessage(	client_socket,
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
		status = READY;
		//Stop RTP Progress
		stopRTPProgress();
	}
	else if (methodIsTeardown(method)){
		//PAUSE method Response Message
		constructResponseMessage(	client_socket,
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

/*Send Not Modified Function
  Variable Definition:
  -- url: the request url except domain name and port number
  -- cseq: cseq number
  -- client_socket: socket connected to the client
  Return Value: NULL
*/
void sendNotModified(char *url, u_int32 cseq, int client_socket){
	//Send Response Message to client
	constructResponseMessage(	client_socket,
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

/*Send Bad Request Function
  Variable Definition:
  -- detail: information about bad request
  -- client_socket: socket connected to the client
  Return Value: NULL
*/
void sendBadRequest(const char *detail, int client_socket){	
	char	*entity_body = (char*)malloc(sizeof(char) * (HALFBUF_SIZE + 1));	//entity body
	
	//Set the entity body
	sprintf(entity_body, "400 Bad Request -- The request could not be understood by the server! (%s)", detail);
	//Send Response Message to client
	constructResponseMessage(	client_socket,
								NULL,
								400,
								"Bad Request",
								0,
								NULL,
								NULL,
								strlen(entity_body) + 1,
								"text/plain",
								entity_body);

	return;
}

/*Send Not Found Function
  Variable Definition:
  -- url: the request url except domain name and port number
  -- client_socket: socket connected to the client
  Return Value: NULL
*/
void sendNotFound(char *url, int client_socket){
	char	*entity_body = (char*)malloc(sizeof(char) * (HALFBUF_SIZE + 1));	//entity body
	
	//Set the entity body
	sprintf(entity_body, "404 Not Found -- The requested URL /%s does not exist on this server!", url);
	//Send Response Message to client
	constructResponseMessage(	client_socket,
								NULL,
								404,
								"Not Found",
								0,
								NULL,
								NULL,
								strlen(entity_body) + 1,
								"text/plain",
								entity_body);

	return;
}

/*Send Method Not Allowed Function
  Variable Definition:
  -- method: the request method
  -- client_socket: socket connected to the client
  Return Value: NULL
*/
void sendMethodNotAllowed(const char *method, int client_socket){
	char	*entity_body = (char*)malloc(sizeof(char) * (HALFBUF_SIZE + 1));	//entity body
	
	//Set the entity body
	sprintf(entity_body, "405 Method Not Allowed -- %s is not allowed!", method);
	//Send Response Message to client
	constructResponseMessage(	client_socket,
								NULL,
								405,
								"Method Not Allowed",
								0,
								NULL,
								NULL,
								strlen(entity_body) + 1,
								"text/plain",
								entity_body);

	return;
}

/*Send Session Not Found Function
  Variable Definition:
  -- client_socket: socket connected to the client
  Return Value: NULL
*/
void sendSessionNotFound(int client_socket){
	char	*entity_body = (char*)malloc(sizeof(char) * (HALFBUF_SIZE + 1));	//entity body
	
	//Set the entity body
	sprintf(entity_body, "454 Session Not Found -- Session should be %u!", session_id);
	//Send Response Message to client
	constructResponseMessage(	client_socket,
								NULL,
								454,
								"Session Not Found",
								0,
								NULL,
								NULL,
								strlen(entity_body) + 1,
								"text/plain",
								entity_body);

	return;
}

/*Send Method Not Valid in This State Function
  Variable Definition:
  -- method: the request method
  -- client_socket: socket connected to the client
  Return Value: NULL
*/
void sendMethodNotValidInThisState(const char *method, int client_socket){
	char	*entity_body = (char*)malloc(sizeof(char) * (HALFBUF_SIZE + 1));	//entity body
	char	*status_string = (char*)malloc(sizeof(char) * (NAME_SIZE + 1));		//status string

	//Set the status_string's value
	switch (status){
		case INIT:
			strcpy(status_string, "INIT");
			break;
		case READY:
			strcpy(status_string, "READY");
			break;
		case PLAYING:
			strcpy(status_string, "PLAYING");
			break;
		default:
			strcpy(status_string, "NULL");
			break;
	}
	//Set the entity body
	sprintf(entity_body, "455 Method Not Valid in This State -- %s is not valid in %s state!", method, status_string);
	//Send Response Message to client
	constructResponseMessage(	client_socket,
								NULL,
								455,
								"Method Not Valid in This State",
								0,
								NULL,
								NULL,
								strlen(entity_body) + 1,
								"text/plain",
								entity_body);

	return;
}

/*Send Unsupported Transport Function
  Variable Definition:
  -- protocol: the request transport protocol
  -- client_socket: socket connected to the client
  Return Value: NULL
*/
void sendUnsupportedTransport(const char *protocol, int client_socket){
	char	*entity_body = (char*)malloc(sizeof(char) * (HALFBUF_SIZE + 1));	//entity body
	
	//Set the entity body
	sprintf(entity_body, "461 Unsupported transport -- %s is not supported by the server!", protocol);
	//Send Response Message to client
	constructResponseMessage(	client_socket,
								NULL,
								461,
								"Unsupported Transport",
								0,
								NULL,
								NULL,
								strlen(entity_body) + 1,
								"text/plain",
								entity_body);

	return;
}

/*Send RTSP Version Not Supported
  Variable Definition:
  -- version: the request RTSP version
  -- client_socket: socket connected to the client
  Return Value: NULL
*/
void sendRTSPVersionNotSupported(const char *version, int client_socket){
	char	*entity_body = (char*)malloc(sizeof(char) * (HALFBUF_SIZE + 1));	//entity body
	
	//Set the entity body
	sprintf(entity_body, "505 RTSP Version Not Supported -- %s is not supported by the server!", version);
	//Send Response Message to client
	constructResponseMessage(	client_socket,
								NULL,
								505,
								"RTSP Version Not Supported",
								0,
								NULL,
								NULL,
								strlen(entity_body) + 1,
								"text/plain",
								entity_body);

	return;
}
