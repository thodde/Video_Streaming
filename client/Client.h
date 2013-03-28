/*
	File Name:		Client.h
	Author:			Trevor Hodde
*/

#ifndef	CLIENT_H_
#define CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <gtk/gtk.h>

#define	CLIENT_RTP_PORT	25000
#define	WINDOW_TITLE	"Streaming Video Player"
#define	PROTOCOL_TYPE	"RTP/UDP"
#define	PROTOCOL_METHOD	"Compression"
#define	MY_NAME			"Trevor Hodde"
#define	VIDEO_ICON		"../icon/video.png"
#define	SCREEN_NAME		"../icon/Screenshot"
#define	CRLF			"\r\n"

enum	widget_size_constants{	//widget size constants
	BORDER_WIDTH_SIZE	= 1,
	BOX_SPACING_SIZE	= 2,
	BOX_PADDING_SIZE	= 3,
	BUTTON_WIDTH_SIZE	= 60,
	BUTTON_LENGTH_SIZE	= 125,
	WINDOW_WIDTH_SIZE	= 500,
	WINDOW_LENGTH_SIZE	= 550,
};
enum	size_constants{			//size constants
	ONE_SIZE			= 1,
	NUMBER_SIZE 		= 4,
	RTP_HEAD_SIZE		= 12,
	HALFSTR_SIZE 		= 64,
	STRING_SIZE 		= 128,
	HALFBUF_SIZE 		= 512,
	BUFFER_SIZE 		= 1024,
	FRAME_SIZE			= 15360,
};
enum	color_constants{		//color constants
	RED_VALUE			= 62194,
	GREEN_VALUE			= 39578,
	BLUE_VALUE			= 30326,
};
enum	status_constants{		//rtsp client status constants
	INIT,
	READY,
	PLAYING,
};
enum	method_constants{		//rtsp method constants
	SETUP,
	PLAY,
	TEARDOWN,
};
enum	rtp_parameters{			//rtp parameters constants
	RTP_VERSION 		= 0x80,
	PADDING				= 0x00,
	EXTENSION			= 0x00,
	CSRC_COUNT			= 0x00,
	MARKER				= 0x00,
	PAYLOAD_TYPE		= 0x1a,
	RTP_VERSION_MASK	= 0xc0,
	PADDING_MASK		= 0x20,
	EXTENSION_MASK		= 0x10,
	CSRC_COUNT_MASK		= 0x0f,
	MARKER_MASK			= 0x80,
	PAYLOAD_TYPE_MASK	= 0x7f,
};

///////////////STRUCT DEFINITION///////////////
/*strcutre of type*/
typedef unsigned char	u_int8;
typedef unsigned short	u_int16;
typedef unsigned int	u_int32;
typedef short			int16;

/*structure of client data*/
typedef struct _client_data{
	GtkWidget		*window;		//window widget
	const gchar		*host;			//server ip address/name
	const gchar		*service;		//server port/service
	const gchar		*video;			//server port/service
}CLIENT_DATA;

/*structure of rtsp response header message*/
typedef struct _rtsp_header{
	char	field_name[HALFSTR_SIZE];		//field name
	char	field_value[HALFBUF_SIZE];		//field value
	struct _rtsp_header *next;				//next pointer
}RTSP_HEADER;

/*structure of rtp packet header*/
typedef struct _rtp_header{
	u_int8		vpxcc;						//rtp parameter: version, padding, extension, and csrc count
	u_int8		mpt;						//rtp parameter: marker, payload type
	u_int16		seq_number;					//sequence number field
	u_int32		timestamp;					//timestamp field
	u_int32		ssrc;						//ssrc field
}RTP_HEADER;

///////////////GLOBAL VARIABLE///////////////
GtkWidget	*toolbar;			//toolbar widget
GtkWidget	*image;				//image widget
GtkWidget	*setupButton;		//setup button widget
GtkWidget	*playButton;		//play button widget
GtkWidget	*teardownButton;	//teardown button widget
u_int32		session_id;			//seesion id number
u_int32		cseq_number;		//cseq number
u_int32		status;				//client status
int			client_rtp_port;	//client rtp port number
int			client_rtcp_port;	//client rtcp port number
int			server_rtp_port;	//server rtp port number

///////////////FUNCTION DECLARATION///////////////
/*ClientUtility.c*/
void	initClient(int port);
int		setupClientTCPSocket(const char *host, const char *service);
int		setupClientUDPSocket(const char *service);
/*AddressUtility.c*/
void	printSocketAddress(const struct sockaddr *address, FILE *stream);
/*HandleUtility.c*/
bool	handleServerResponse(GtkWidget *widget, int client_socket);
RTSP_HEADER		*getHeaderLines(FILE *stream);
char	*getResponseContents(FILE *stream);
/*RTPPacketUtility.c*/
void	startRTPProgress(CLIENT_DATA *client_data);
void	stopRTPProgress(u_int32 status_signal);
bool	checkRTPHeader(const u_int8 *rtp);
void	SIGIOHandler(int signal);
/*LayoutUtility.c*/
void	initClientLayout(GtkWidget *window);
GtkWidget	*getMenubarMenu(GtkWidget *window);
GtkWidget	*getToolbar();
GtkWidget	*getButton(	GtkWidget 	*box,
						const gchar *icon_name,
						const gchar *label_string,
						const gchar *tooltip_label);
void	setSensitive(	gboolean setup_s,
						gboolean play_s,
						gboolean teardown_s);
void	setImage(const u_int8 *rtp);
GdkPixbuf	*getIcon(const gchar *icon_name);
/*CallbackUtility.c*/
void	initCallback(CLIENT_DATA *client_data);
void	menuCallback(	gpointer	callback_data,
						guint		callback_action,
						GtkWidget	*menu_item);
void	setupRTSPCallback(GtkWidget *widget, CLIENT_DATA *client_data);
void	playRTSPCallback(GtkWidget *widget, CLIENT_DATA *client_data);
void	teardownRTSPCallback(GtkWidget *widget, CLIENT_DATA *client_data);
void	showToolbar(	gpointer	callback_data,
						guint		callback_action,
						GtkWidget	*menu_item);
void	showAboutCallback(GtkWidget *widget, gpointer data);
void	showInfoCallback(GtkWidget *widget, gpointer data);
void	showErrorCallback(GtkWidget *widget, gpointer data);
void	showQuesCallback(GtkWidget *widget, gpointer data);
void	showWarnCallback(GtkWidget *widget, gpointer data);
void	enterButtonCallback(GtkWidget *widget, gpointer data);
/*StringUtility.c*/
char	*itoa(int number);
bool	fieldExist(RTSP_HEADER *header, const char *field_name, char *field_value);
char	*splitNameAndValue(char *header_line, const char stop);

#endif //CLIENT_H
