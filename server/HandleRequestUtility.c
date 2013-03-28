/////////////////////////////////////////////////////////////////
/*
	File Name:		HandleRequestUtility.c
	Author:			Trevor Hodde
	Note:			This HandleRequestUtility.c file includes 
					Handle Client Request Functions.
*/
/////////////////////////////////////////////////////////////////

///////////////HEADER FILES///////////////
#include "Server.h"

///////////////FUNCTIONS///////////////
/*Handle Client Request Function
  Variable Definition:
  -- client_socket: socket connected to the client
  Return Value: NULL
*/
void handleClientRequest(int client_socket){
	FILE 	*channel;						//file stream for client socket
	char 	request_line[STRING_SIZE];		//client request line
	char	*return_value;					//each line header pointer
	
	//Initialize request_line buffer
	memset(request_line, 0, STRING_SIZE);
	//Create an input stream from the socket
	channel = fdopen(client_socket, "r");
	if (channel == NULL){
		dieWithSystemMessage("fdopen() failed");
	}
	
	//Get client Request Line (Jump the blank line)
	do{
		return_value = fgets(request_line, STRING_SIZE, channel);
	}while (syntaxChecking(return_value, BLANK_LINE));
	//Output the client_socket id and Request Line
	printf("Got a call on %d: request = %s", client_socket, request_line);
	//Get client Header Lines & Response the client request
	respondClientRequest(request_line, getHeaderLines(channel), client_socket);
	
	//Close client socket
	close(client_socket);
	//Close file stream
	fclose(channel);

	return;
}

/*Respond Client Request Function
  Variable Definition:
  -- request: client request line
  -- header: client header lines
  -- client_socket: socket connected to the client
  Return Value: NULL
*/
void respondClientRequest(char *request, RTSP_HEADER *header, int client_socket){
	char 		method[STRING_SIZE];			//method field: SETUP, PLAY, PAUSE, or TEARDOWN
	char 		url[STRING_SIZE];				//url field: for example, rtsp://localhost:5678/movie.Mjpeg
	char 		version[STRING_SIZE];			//rtsp version field: RTSP/1.0
	char		field_value[HALFBUF_SIZE];		//field value string
	u_int32		cseq_number = 0;				//cseq number
	
	//Initialize method, url, version, and field_value buffer
	memset(method, 0, STRING_SIZE);
	memset(url, 0, STRING_SIZE);
	memset(version, 0, STRING_SIZE);
	memset(field_value, 0, HALFBUF_SIZE);
	
	//Test the client RTSP Request Line
	if (!syntaxChecking(request, REQUEST_LINE)){
		//400 Bad Request: the request could not be understood by the server
		sendBadRequest("Request Line is syntactically incorrect!", client_socket);
		return;
	}
	
	//Get the method, url, and rtsp version
	sscanf(request, "%s%s%s", method, url, version);
	//Decode the URL(if it has %HEX code)
	decodeURL(url);

	//Test the method
	if (methodNotAllow(method)){
		//405 Method Not Allowed: the method field is neither "SETUP", "PLAY", "PAUSE" nor "TEARDOWN"
		sendMethodNotAllowed(method, client_socket);
		return;
	}
	//Test the Requested URL
	else if (!syntaxChecking(url, URL_FORMAT)){
		//400 Bad Request: the request could not be understood by the server
		sendBadRequest("Requested URL is syntactically incorrect!", client_socket);
		return;
	}
	//Test the RTSP version
	else if (!syntaxChecking(version, RTSP_VERSION)){
		//400 Bad Request: the request could not be understood by the server
		sendBadRequest("RTSP Version is syntactically incorrect!", client_socket);
		return;
	}
	//Test the RTSP version 1.0
	else if (!syntaxChecking(version, RTSP_VERSION_1)){
		//505 RTSP Version Not Supported: the requested RTSP protocol version is not supported by server
		sendRTSPVersionNotSupported(version, client_socket);
		return;
	}

#ifdef	DEBUG
	RTSP_HEADER		*debug_header_node;

	DEBUG_START;
	fputs("RTSP request header lines:\n", stdout);
	//Output the RTSP request header lines
	for (debug_header_node = header->next; debug_header_node != NULL; debug_header_node = debug_header_node->next){
		fputs(debug_header_node->field_name, stdout);
		fputs(": ", stdout);
		fputs(debug_header_node->field_value, stdout);
		fputc('\n', stdout);
	}
	DEBUG_END;
#endif

	//Test the Header Line
	if (headerLinesIncorrect(header, field_value)){
		//400 Bad Request: the request could not be understood by the server
		sendBadRequest(field_value, client_socket);
		return;
	}
	//Test the "CSeq" field
	else if (fieldNotExist(header, "cseq", field_value)){
		//400 Bad Request: the request could not be understood by the server
		sendBadRequest("CSeq field does not exist!", client_socket);
		return;
	}
	//Test the "Session" field
	else if ((!methodIsSetup(method)) && fieldNotExist(header, "session", field_value)){
		//400 Bad Request: the request could not be understood by the server
		sendBadRequest("Session field does not exist!", client_socket);
		return;
	}
	//Test the "Transport" field
	else if (methodIsSetup(method) && fieldNotExist(header, "transport", field_value)){
		//400 Bad Request: the request could not be understood by the server
		sendBadRequest("Transport field does not exist!", client_socket);
		return;
	}
	//Test the "Range" field
	else if ((!(fieldNotExist(header, "range", field_value)))
				&& (!syntaxChecking(field_value, RANGE_FORMAT))){
		//400 Bad Request: the request could not be understood by the server
		sendBadRequest("Range field value(npt=number[-number]) is syntactically incorrect!", client_socket);
		return;
	}
	//Test the "If-Modified-Since" field
	else if ((!(fieldNotExist(header, "if-modified-since", field_value)))
				&& (!syntaxChecking(field_value, TIME_FORMAT))){
		//400 Bad Request: the request could not be understood by the server
		sendBadRequest("If-Modified-Since field value(time format) is syntactically incorrect!", client_socket);
		return;
	}
	
	//Now we are sure that the request message is SYNTACTICALLY CORRECT
	//Get the Requested File or Directory's name
	pathBelowCurrentDirectory(url);
	printf("Requested File or Directory is %s\n", url);
	
	//Test the requested file on the server
	if (urlNotExist(url)){
		//404 Not Found: the requested document does not exist on this server
		sendNotFound(url, client_socket);
		return;
	}
	//Test the requested url is a directory
	else if (urlIsADirectory(url)){
		//404 Not Found: the requested document does not exist on this server
		sendNotFound(url, client_socket);
		return;
	}
	//Test the method is valid in special state
	else if (methodIsNotValidInState(method)){
		//455 Method is not valid in this state: the method is not valid in this state
		sendMethodNotValidInThisState(method, client_socket);
		return;
	}

	//Get the RTSP Request Message information
	cseq_number = getRTSPInfo(header);

	//Test the "Session" field's value
	if (cseq_number == 0){
		//454 Session Not Found: the session id is not equal to the server's
		sendSessionNotFound(client_socket);
		return;
	}
	//Test the protocol type
	else if (strcmp(protocol_type, PROTOCOL_TYPE) != 0){
		//461 Unsupported Transport: the transport protocol is not supported by the server
		sendUnsupportedTransport(protocol_type, client_socket);
		return; 
	}
	//Test the "If-Modified-Since" field
	else if (fieldNotExist(header, "if-modified-since", field_value)){
		//200 OK: the request is good
		sendOK(url, method, cseq_number, client_socket);
		return;
	}
	//Test the "If-Modified-Since" field value
	else if (compareModifiedTime(url, field_value)){
		//304 Not Modified: the request does not Modified since If-Modified-Since field
		sendNotModified(url, cseq_number, client_socket);
		return;
	}
	else{
		//200 OK: the request is good
		sendOK(url, method, cseq_number, client_socket);
		return;
	}
}

/*Construct Response Message to Client Function
  Variable Definition:
  -- client_socket: socket connected to the client
  -- stream: file stream of socket or server file
  -- status_code: response status code
  -- status_message: response status message
  -- cseq_number: cseq number
  -- transport: transport field
  -- last_modified_time: response last modified time field
  -- content_length: response content length field
  -- content_type: response content type field
  -- content: response entity body field
  Return Value: bytes of whole message
*/
int	constructResponseMessage(	int				client_socket,
								FILE 			**stream,
								int				status_code,
								const char		*status_message,
								u_int32			cseq_number,
								const char		*transport,
								const char		*last_modified_time,
								int				content_length,
								const char		*content_type,
								const char		*content){
	FILE	*channel;		//file stream for client socket
	int		bytes = 0;		//bytes that send to client
	
	//Create an output stream to the socket
	channel = fdopen(client_socket, "w");
	if (channel == NULL){
		dieWithSystemMessage("fopen() failed");
	}

	//Response Message Status Line
	bytes = fprintf(channel, "RTSP/1.0 %d %s%s", status_code, status_message, CRLF);
	//Response Cseq field
	if (cseq_number != 0){
		bytes += fprintf(channel, "Cseq: %u%s", cseq_number, CRLF);
	}
	//Response Session field
	if (session_id != 0){
		bytes += fprintf(channel, "Session: %u%s", session_id, CRLF);
	}
	//Response Transport field
	if (transport){
		bytes += fprintf(channel, "Transport: %s%s", transport, CRLF);
	}
	//Response Date field
	bytes += fprintf(channel, "Date: %s%s", convertTimeFormat(getTimeInGMTFormat(NULL, 0), 1), CRLF);
	//Response Server field
	bytes += fprintf(channel, "Server: Multimedia Networks: RTSP server by %s (Unix)%s", MY_NAME, CRLF);
	//Response Last-Modified field
	if (last_modified_time){
		bytes += fprintf(channel, "Last-Modified: %s%s", last_modified_time, CRLF);
	}
	//Response Content-Length field
	if (content_length != 0){
		bytes += fprintf(channel, "Content-Length: %d%s", content_length, CRLF);
	}
	//Response Message Content-Type field
	if (content_type){
		bytes += fprintf(channel, "Content-Type: %s%s%s", content_type, CRLF, CRLF);
	}
	//Response Message Entity Body field
	if (content){
		bytes += fprintf(channel, "%s%s", content, CRLF);
	}
	//Flush the channel
	fflush(channel);
	
	//Pass the stream
	if (stream){
		*stream = channel;
	}
	else{
		//Close file stream
		fclose(channel);
	}
	
	return bytes;
}

/*Get Header Lines Function
  Variable Definition:
  -- stream: file stream for client socket
  Return Value: header pointer of header lines structure
*/
RTSP_HEADER *getHeaderLines(FILE *stream){
	RTSP_HEADER 	*header;					//_rtsp_header structure head pointer
	RTSP_HEADER 	*node;						//_rtsp_header structure node
	char 			buffer[HALFBUF_SIZE];		//header line string
	char 			*field_name;				//header field name
	int 			number = 0;					//number of header lines
	
	//Allocate memory for _rtsp_header structure header node
	header = (RTSP_HEADER*)malloc(sizeof(RTSP_HEADER));
	//Initialize buffer
	memset(buffer, 0, HALFBUF_SIZE);
	//Initialize the structure _rtsp_header head node
	strcpy(header->header_line, "");
	strcpy(header->field_name, "RTSP Request Header Lines");
	strcpy(header->field_value, "0");
	header->next = NULL;
	
	//Let the node pointer point to the header
	node = header;
	//Get the header lines
	while (fgets(buffer, HALFBUF_SIZE, stream) != NULL && strcmp(buffer, CRLF) != 0){
		//Allocate the memory for new node
		node->next = (RTSP_HEADER*)malloc(sizeof(RTSP_HEADER));
		//Let the node pointer point to the current header line
		node = node->next;
		//Assign the variable(header_line)
		strcpy(node->header_line, buffer);
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

/*Get RTSP Client Request Information Function
  Variable Definition:
  -- header: _rtsp_header structure header pointer
  Return Value: if session value is incorrect, return 0, else return cseq number
*/
u_int32 getRTSPInfo(RTSP_HEADER *header){
	RTSP_HEADER		*node;					//_rtsp_header structure node
	char			*number_string;			//client number string (including port number and range number)
	u_int32			cseq_number = 0;		//cseq number

	//Test the session_id value
	if (session_id == 0){
		//Create a session_id
		session_id = getRandomNumber(BASIC_NUMBER, MOD_NUMBER);
	}
	//Search the field_name in the RTSP Request Message header lines
	for (node = header->next; node != NULL; node = node->next){
		//Find the "Cseq" field
		if (strcmp(node->field_name, "cseq") == 0){
			cseq_number = atoi(node->field_value);
		}
		//Find the "Session" field
		else if (strcmp(node->field_name, "session") == 0){
			//Handle the unequal session id
			if (session_id != atoi(node->field_value)){
				cseq_number = 0;
				break;
			}
		}
		//Find the "Transport" field
		else if (strcmp(node->field_name, "transport") == 0){
			//Set the protocol type value
			strcpy(protocol_type, splitNameAndValue(node->field_value, ';'));
			//Set the protocol method value
			strcpy(protocol_method, splitNameAndValue(node->field_value, ';'));
			//Find the client rtp port number value
			number_string = splitNameAndValue(node->field_value, ';');
			//Get the client rtp port number
			splitNameAndValue(number_string, '=');
			//Set the client rtp port number
			client_rtp_port = atoi(number_string);
			//Set the client rtcp port number
			client_rtcp_port = client_rtp_port + ONE_SIZE;
		}
		//Find the "Range" field (for example "npt=0-" or npt=17)
		else if (strcmp(node->field_name, "range") == 0){
			//Get the range value (for example "0-" or "17")
			splitNameAndValue(node->field_value, '=');
			//Test the range field contain "-" character
			if (strstr(node->field_value, "-") != NULL){
				//Set the range start number
				range_start = atoi(splitNameAndValue(node->field_value, '-'));
				//Set the range end number
				if (strcmp(node->field_value, "") == 0){
					//-1 means infinite
					range_end = -1;
				}
				else{
					range_end = atoi(node->field_value);
				}
			}
			//Range field does not contain '-' character
			else{
				//Only set the range end value
				range_end = atoi(node->field_value);
			}
		}
	}

	return cseq_number;
}

/*Generate a Random Integer to Obtain Session ID or SSRC Function
  Variable Definition:
  -- bas_number: basic number
  -- mod_number: mod number
  Return Value: session id or SSRC
*/
u_int32 getRandomNumber(int bas_number, u_int32 mod_number){
	struct timeval	*random_time;		//timeval structure
	u_int32			ran_number;			//random number

	//Allocate memory for timeval structure
	random_time = (struct timeval*)malloc(sizeof(struct timeval));
	//Get the current time (including seconds and microseconds)
	gettimeofday(random_time, NULL);
	//Generate the random seed
	srand((unsigned)random_time->tv_usec);
	//Get a random number between 10000000 and 10000000 + mod_number
	ran_number = bas_number + rand() % mod_number;
	//Free timeval structure
	free(random_time);
	
	return ran_number;
}
