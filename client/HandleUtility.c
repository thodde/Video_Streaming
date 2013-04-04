/**
 * File Name: HandleUtility.c
 * Author:    Trevor Hodde
 * Most of the RTP Header code was borrowed from:
 * cs.columbia.edu/irt/software/rtplib
 */

#include "Client.h"

bool handleServerResponse(GtkWidget *widget, int client_socket) {
	RTSP_HEADER	*header;
	FILE 	*channel;
	char 	response_line[STRING_SIZE];
	char	version[STRING_SIZE];
	char	status_code[NUMBER_SIZE];
	char	status_message[STRING_SIZE];
	char	field_value[HALFBUF_SIZE];
	char	*content;		
	char	*p;
	int	count;
	
	//initialize all the strings to be empty
	memset(response_line, 0, STRING_SIZE);
	memset(version, 0, STRING_SIZE);
	memset(status_code, 0, NUMBER_SIZE);
	memset(status_message, 0, STRING_SIZE);
	memset(field_value, 0, HALFBUF_SIZE);

	//Create an input stream from the socket
	channel = fdopen(client_socket, "r");

	if (channel == NULL) {
		showErrorCallback(widget, "Unable to create received channel!");
		return false;
	}
	
	//Get server Response Line
	if (fgets(response_line, STRING_SIZE, channel) == NULL) {
		showErrorCallback(widget, "Unable to receive RTSP Response!");
		return false;
	}

	//Output the client_socket id and Response Line
	printf("Call on %d: response = %s", client_socket, response_line);
	//Get server Header Lines
	header = getHeaderLines(channel);

	//Get server response content
	content = getResponseContents(channel);

	//Get the rtsp version, status code, and status message
	sscanf(response_line, "%s%s%s", version, status_code, status_message);

	//Test the status code is neither 200 (OK) nor 304 (Not Modified)
	if ((strcmp(status_code, "200") != 0) && (strcmp(status_code, "304") != 0)) {
		showErrorCallback(widget, content);
		return false;
	}
	
	//Test the session field
	if (fieldExist(header, "session", field_value)) {
		//Set the session id
		session_id = atoi(field_value);
	}

	//Test the transport field
	if (fieldExist(header, "transport", field_value)) {
		//Remove the protocol type, protocol method, client port, and server port
		for (count = 0; count < NUMBER_SIZE; count++) {
			p = splitNameAndValue(field_value, ';');
		}

		//Get the server rtp port number
		splitNameAndValue(p, '=');
		//Set the server rtp port number
		server_rtp_port = atoi(p);
	}
	//Close file stream
	fclose(channel);

	return true;
}

RTSP_HEADER *getHeaderLines(FILE *stream){
	char 	buffer[HALFBUF_SIZE];
	char 	*field_name;	
	int 	number = 0;
	RTSP_HEADER 	*header; 
	RTSP_HEADER 	*node;	
	
	//Allocate memory for _rtsp_header structure header pointer
	header = (RTSP_HEADER*)malloc(sizeof(RTSP_HEADER));
	//Initialize buffer
	memset(buffer, 0, HALFBUF_SIZE);
	//Initialize the structure _rtsp_header header node
	strcpy(header->field_name, "RTSP Response Header Lines");
	strcpy(header->field_value, "0");
	header->next = NULL;
	
	//Let the node pointer point to the header
	node = header;
	//Get the header lines
	while (fgets(buffer, HALFBUF_SIZE, stream) != NULL && strcmp(buffer, CRLF) != 0) {
		//Allocate the memory for new node
		node->next = (RTSP_HEADER*)malloc(sizeof(RTSP_HEADER));
		//Let the node pointer point to the current header line
		node = node->next;
		//Split the Field Name and Field Value (According to the ':')
		field_name = splitNameAndValue(buffer, ':');
		//Assign the variable(field_name and field_value)
		strcpy(node->field_name, field_name);
		strcpy(node->field_value, buffer);
		node->next = NULL;
		//Count the number of header lines
		number++;
	}

	//Set the value of header pointer's field_value (the number of header lines)
	strcpy(header->field_value, itoa(number));
	
	return header;
}

char *getResponseContents(FILE *stream) {
	char	*content = (char*)malloc(sizeof(char) * (BUFFER_SIZE + 1));
	char	buffer[BUFFER_SIZE];

	//Initialize buffer
	memset(buffer, 0, HALFBUF_SIZE);
	//Initialize content
	strcpy(content, buffer);
	//Get the response content
	while (fgets(buffer, BUFFER_SIZE, stream) != NULL) {
		strcat(content, buffer);
	}

	return content;
}
