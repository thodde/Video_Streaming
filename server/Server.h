////////////////////////////////////////////////////////////////
/*
	File Name:		Server.h
	Author:			Trevor Hodde
	Note:			This Server.h file includes
					HEADER FILES, MACRO, STRUCT DEFINITION,
					GLOBAL VARIABLE AND FUNCTION DECLARATION.
*/
////////////////////////////////////////////////////////////////

///////////////PRECOMPILER///////////////
#ifndef	SERVER_H_
#define SERVER_H_

///////////////DEBUG///////////////
#define DEBUG 1
#ifdef DEBUG
	#define DEBUG_PRINT		printf("%s-%s:%d:", __FILE__, __FUNCTION__, __LINE__)
	#define	DEBUG_START		fputs("/***********DEBUG INFORMATION***********/\n", stdout)
	#define DEBUG_END		fputs("/******************END******************/\n", stdout)
#else
	#define DEBUG_PRINT
	#define DEBUG_START
	#define DEBUG_END
#endif

///////////////HEADER FILES///////////////
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

///////////////MACRO///////////////
#define	MAX_PENDING		10
#define	SERVER_RTP_PORT	30000
#define	FRAME_PERIOD	70000
#define	BASIC_NUMBER	10000000
#define	MOD_NUMBER		89999999
#define	PROTOCOL_TYPE	"rtp/udp"
#define	MY_NAME			"Trevor Hodde"
#define	CRLF			"\r\n"

///////////////CONSTANTS VARIABLES///////////////
enum	size_constants{			//size constants
	ONE_SIZE		= 1,
	NUMBER_SIZE 	= 5,
	RTP_HEAD_SIZE	= 12,
	NAME_SIZE 		= 16,
	TIME_SIZE 		= 32,
	HALFSTR_SIZE 	= 64,
	STRING_SIZE 	= 128,
	HALFBUF_SIZE 	= 512,
	BUFFER_SIZE 	= 1024,
};
enum	syntax_constants{		//syntax checking constants
	BLANK_LINE,
	REQUEST_LINE,
	URL_FORMAT,
	RTSP_VERSION,
	RTSP_VERSION_1,
	HEADER_LINE,
	RANGE_FORMAT,
	TIME_FORMAT,
};
enum	status_constants{		//rtsp server status constants
	INIT,
	READY,
	PLAYING,
};
enum	rtp_parameters{			//rtp parameters constants
	RTP_VERSION 	= 0x80,
	PADDING			= 0x00,
	EXTENSION		= 0x00,
	CSRC_COUNT		= 0x00,
	MARKER			= 0x00,
	PAYLOAD_TYPE	= 0x1a,
};

///////////////STRUCT DEFINITION///////////////
/*strcutre of type*/
typedef unsigned char	u_int8;
typedef unsigned short	u_int16;
typedef unsigned int	u_int32;
typedef short			int16;

/*structure of arguments to pass to client thread*/
typedef struct _thread_arguments{
	int		client_socket;						//socket descriptor for client
}THREAD_ARGUMENTS;

/*structure of udp socket information*/
typedef struct _udp_socket_info{
	int					socket;					//udp socket descriptor for server
	struct addrinfo		*address;				//socket address
}UDP_SOCKET_INFO;

/*structure of rtsp request header message*/
typedef struct _rtsp_header{
	char	header_line[HALFBUF_SIZE];			//header line
	char	field_name[HALFSTR_SIZE];			//field name
	char	field_value[HALFBUF_SIZE];			//field value
	struct _rtsp_header *next;					//next pointer
}RTSP_HEADER;

/*structure of rtp packet header*/
typedef struct _rtp_header{
	u_int8		vpxcc;							//rtp parameter: version, padding, extension, and csrc count
	u_int8		mpt;							//rtp parameter: marker, payload type
	u_int16		seq_number;						//sequence number field
	u_int32		timestamp;						//timestamp field
	u_int32		ssrc;							//ssrc field
}RTP_HEADER;

///////////////GLOBAL VARIABLES///////////////
char		protocol_type[NAME_SIZE];			//client protocol type: rtp/udp
char		protocol_method[NAME_SIZE];			//client protocol method: compression
char		rtp_address[INET6_ADDRSTRLEN];		//client rtp address
u_int32		session_id;							//server session id: eight digits
u_int32		status;								//server status
u_int32		range_start;						//range start number
int			range_end;							//range end number
int			client_rtp_port;					//client rtp port number
int			client_rtcp_port;					//client rtcp port number
int			server_rtp_port;					//server rtp port number

///////////////FUNCTION DECLARATION///////////////
/*ServerUtility.c*/
void	initServer(int port);
int 	setupServerTCPSocket(const char *service);
UDP_SOCKET_INFO		*setupServerUDPSocket(const char *host, const char *service);
int 	acceptTCPConnection(int server_socket);
/*AddressUtility.c*/
void 	printSocketAddress(const struct sockaddr *address, FILE *stream, bool get_address_signal);
/*Thread.c*/
void	*threadMain(void *thread_arguments);
/*HandleRequestUtility.c*/
void 	handleClientRequest(int client_socket);
void 	respondClientRequest(char *request, RTSP_HEADER *header, int client_socket);
int		constructResponseMessage(	int				client_socket,
									FILE			**stream,
									int				status_code,
									const char		*status_message,
									u_int32			cseq_number,
									const char		*transport,
									const char		*last_modified_time,
									int				content_length,
									const char		*content_type,
									const char		*content);
RTSP_HEADER		*getHeaderLines(FILE *stream);
u_int32	getRTSPInfo(RTSP_HEADER *header);
u_int32	getRandomNumber(int bas_number, u_int32 mod_number);
/*RTPPacketUtility.c*/
u_int32	getVideoInfo(char *url);
void	startRTPProgress();
void	stopRTPProgress();
void	closeVideoStream();
void	setRTPPacketHeader(RTP_HEADER *rtp);
u_int8	*constructRTPPacket(size_t *length);
void	catchAlarm(int ignored);
/*SendUtility.c*/
void	sendOK(char *url, const char *method, u_int32 cseq, int client_socket);
void	sendNotModified(char *url, u_int32 cseq, int client_socket);
void 	sendBadRequest(const char *detail, int client_socket);
void	sendNotFound(char *url, int client_socket);
void	sendMethodNotAllowed(const char *method, int client_socket);
void	sendSessionNotFound(int client_socket);
void	sendMethodNotValidInThisState(const char *method, int client_socket);
void	sendUnsupportedTransport(const char *protocol, int client_socket);
void	sendRTSPVersionNotSupported(const char *version, int client_socket);
/*FileUtility.c*/
char	*getFileType(char *url);
/*TimeUtility.c*/
void	setTimer(	struct itimerval	timer,
					int					type,
					u_int32				interval_sec,
					u_int32				interval_usec,
					u_int32				value_sec,
					u_int32				value_usec);
struct tm	*getTimeInGMTFormat(char *url, int signal_value);
char	*convertTimeFormat(struct tm *gmt_time, int signal_value);
bool	compareModifiedTime(char *url, char *modified_time_string);
/*StringUtility.c*/
char 	*itoa(int number);
void	decodeURL(char *url);
void	pathBelowCurrentDirectory(char *url);
bool	methodNotAllow(char *method);
bool	headerLinesIncorrect(RTSP_HEADER *header, char *field_value);
bool	urlNotExist(char *url);
bool	fieldNotExist(RTSP_HEADER *header, const char *field_name, char *field_value);
bool	urlIsADirectory(char *url);
bool	methodIsSetup(const char *method);
bool	methodIsPlay(const char *method);
bool	methodIsPause(const char *method);
bool	methodIsTeardown(const char *method);
bool	methodIsNotValidInState(const char *method);
bool	syntaxChecking(char *string, int signal_value);
char	*splitNameAndValue(char *header_line, const char stop);

#endif //SERVER_H
