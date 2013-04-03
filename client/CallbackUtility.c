/**
 * File Name: CallbackUtility.c
 * Author:    Trevor Hodde
 */

#include "Client.h"

static CLIENT_DATA *client;

void initCallback(CLIENT_DATA *client_data){
	GtkToolItem	*tool_item;

	//Set a handler for enter that change the button's background color
	g_signal_connect(setupButton, "enter", G_CALLBACK(enterButtonCallback), NULL);
	g_signal_connect(playButton, "enter", G_CALLBACK(enterButtonCallback), NULL);
	g_signal_connect(teardownButton, "enter", G_CALLBACK(enterButtonCallback), NULL);
	//Set a handler for clicked that handle RTSP request & response message
	g_signal_connect(setupButton, "clicked", G_CALLBACK(setupRTSPCallback), client_data);
	g_signal_connect(playButton, "clicked", G_CALLBACK(playRTSPCallback), client_data);
	g_signal_connect(teardownButton, "clicked", G_CALLBACK(teardownRTSPCallback), client_data);
	//Set a handler for clicked that handle RTSP request & response message
	tool_item = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), SETUP);
	g_signal_connect(tool_item, "clicked", G_CALLBACK(setupRTSPCallback), client_data);
	tool_item = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), PLAY);
	g_signal_connect(tool_item, "clicked", G_CALLBACK(playRTSPCallback), client_data);
	tool_item = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), TEARDOWN);
	g_signal_connect(tool_item, "clicked", G_CALLBACK(teardownRTSPCallback), client_data);
	//Set a handler for delete_event that exits GTK
	tool_item = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), 5);

	//Set the global variable
	client = client_data;

	return;
}

void menuCallback(gpointer callback_data, guint callback_action, GtkWidget *menu_item){
	//According to the callback_action, call the special function
	switch (callback_action){
		case SETUP:
			//SETUP callback function
			setupRTSPCallback(menu_item, client);
			break;
		case PLAY:
			//PLAY callback function
			playRTSPCallback(menu_item, client);
			break;
		case TEARDOWN:
			//TEARDOWN callback function
			teardownRTSPCallback(menu_item, client);
			break;
		default:
			break;
	}

	return;
}

/*SETUP method Callback Function
  Variable Definition:
  -- widget: callback function widget
  -- client_data: callback function widget
  Return Value: NULL
*/
void setupRTSPCallback(GtkWidget *widget, CLIENT_DATA *client_data){
	gchar		*buffer;					//request message buffer
	gint		client_socket = -1;			//socket descriptor for client
	gint		bytes = 0;					//number of bytes
	size_t		message_length;				//request message length

	//Create a connected TCP socket
	client_socket = setupClientTCPSocket(client_data->host, client_data->service);
	if (client_socket < 0){
		showErrorCallback(client_data->window, "setupClientSocket() failed: unable to connect!");
		return;
	}
	//Initialize request message buffer
	buffer = g_malloc(sizeof(gchar) * (BUFFER_SIZE));
	//Increase the cseq number
	cseq_number++;
	//Constuct RTSP SETUP request message
	sprintf(buffer, "SETUP rtsp://%s:%s/%s RTSP/1.0%s"	\
					"CSeq: %u%s"	\
					"Transport: %s; %s; client_port=%d; mode=PLAY%s"	\
					"User-Agent: Multimedia Networks: RTSP client by %s (Unix)%s%s",	client_data->host,
																								client_data->service,
																								client_data->video, CRLF,
																								cseq_number, CRLF,
																								PROTOCOL_TYPE, PROTOCOL_METHOD,
																								client_rtp_port, CRLF,
																								MY_NAME, CRLF, CRLF);

	//Set the buffer length
	message_length = strlen(buffer);
	//Send SETUP request message to the server
	bytes = send(client_socket, buffer, message_length, MSG_NOSIGNAL);
	//Test the send is successful
	if (bytes < 0){
		showErrorCallback(client_data->window, "send() failed");
		return;
	}
	else if (bytes != message_length){
		showErrorCallback(client_data->window, "send() error: sent unexpected number of bytes!");
		return;
	}

	//Now, start receiving the RTSP response message
	//Handle RTSP server response message
	if (handleServerResponse(client_data->window, client_socket)){
		//Set the client status
		status = READY;
		//Set the buttons', toolbars', and menus' sensitive property
		setSensitive(FALSE, TRUE, TRUE);
	}
	//Close client socket
	close(client_socket);

	return;
}

/*PLAY method Callback Function
  Variable Definition:
  -- widget: callback function widget
  -- client_data: callback function widget
  Return Value: NULL
*/
void playRTSPCallback(GtkWidget *widget, CLIENT_DATA *client_data){
	gchar		*buffer;					//request message buffer
	gint		client_socket = -1;			//socket descriptor for client
	gint		bytes = 0;					//number of bytes
	size_t		message_length;				//request message length

	//Create a connected TCP socket
	client_socket = setupClientTCPSocket(client_data->host, client_data->service);
	if (client_socket < 0){
		showErrorCallback(client_data->window, "setupClientSocket() failed: unable to connect!");
		return;
	}
	//Initialize request message buffer
	buffer = g_malloc(sizeof(gchar) * (BUFFER_SIZE));
	//Increase the cseq number
	cseq_number++;
	//Constuct RTSP PLAY request message
	sprintf(buffer, "PLAY rtsp://%s:%s/%s RTSP/1.0%s"	\
					"CSeq: %u%s"	\
					"Session: %u%s"	\
					"User-Agent: Multimedia Networks: RTSP client by %s (Unix)%s%s",	client_data->host,
																								client_data->service,
																								client_data->video, CRLF,
																								cseq_number, CRLF,
																								session_id, CRLF,
																								MY_NAME, CRLF, CRLF);

	//Set the buffer length
	message_length = strlen(buffer);
	//Send PLAY request message to the server
	bytes = send(client_socket, buffer, message_length, MSG_NOSIGNAL);
	//Test the send is successful
	if (bytes < 0){
		showErrorCallback(client_data->window, "send() failed");
		return;
	}
	else if (bytes != message_length){
		showErrorCallback(client_data->window, "send() error: sent unexpected number of bytes!");
		return;
	}

	//Now, start receiving the RTSP response message
	//Handle Server Response Message
	if (handleServerResponse(client_data->window, client_socket)){
		//Set the client status
		status = PLAYING;
		//Set the buttons', toolbars' and menus' sensitive property
		setSensitive(FALSE, FALSE, TRUE);
	}
	//Close client socket
	close(client_socket);

	//Start RTP progress
	startRTPProgress(client_data);

	return;
}

void teardownRTSPCallback(GtkWidget *widget, CLIENT_DATA *client_data){
	gchar		*buffer;					//request message buffer
	gint		client_socket = -1;			//socket descriptor for client
	gint		bytes = 0;					//number of bytes
	size_t		message_length;				//request message length

	//Create a connected TCP socket
	client_socket = setupClientTCPSocket(client_data->host, client_data->service);
	if (client_socket < 0){
		showErrorCallback(client_data->window, "setupClientSocket() failed: unable to connect!");
		return;
	}
	//Initialize request message buffer
	buffer = g_malloc(sizeof(gchar) * (BUFFER_SIZE));
	//Increase the cseq number
	cseq_number++;
	//Constuct RTSP PLAY request message
	sprintf(buffer, "TEARDOWN rtsp://%s:%s/%s RTSP/1.0%s"	\
					"CSeq: %u%s"	\
					"Session: %u%s"	\
					"User-Agent: Multimedia Networks: RTSP client by %s (Unix)%s%s",	client_data->host,
																								client_data->service,
																								client_data->video, CRLF,
																								cseq_number, CRLF,
																								session_id, CRLF,
																								MY_NAME, CRLF, CRLF);

	//Set the buffer length
	message_length = strlen(buffer);
	//Send TEARDOWN request message to the server
	bytes = send(client_socket, buffer, message_length, MSG_NOSIGNAL);
	//Test the send is successful
	if (bytes < 0){
		showErrorCallback(client_data->window, "send() failed");
		return;
	}
	else if (bytes != message_length){
		showErrorCallback(client_data->window, "send() error: sent unexpected number of bytes!");
		return;
	}

	//Now, start receiving the RTSP response message
	//Handle Server Response Message
	if (handleServerResponse(client_data->window, client_socket)){
		//Set the client status
		status = INIT;
		//Set the buttons', toolbars and menus' sensitive property
		setSensitive(TRUE, FALSE, FALSE);
	}
	//Close client socket
	close(client_socket);

	//Stop RTP progress
	stopRTPProgress(status);

	return;
}

void showErrorCallback(GtkWidget *widget, gpointer data){
	GtkWidget	*dialog;		//dialog widget

	//Create a error dialog
	dialog = gtk_message_dialog_new(GTK_WINDOW(widget),
									GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_ERROR,
									GTK_BUTTONS_OK,
									"%s", (gchar*)data);
	//Set the title of dialog
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	//Run the message dialog
	gtk_dialog_run(GTK_DIALOG(dialog));
	//Destroy the message dialog
	gtk_widget_destroy(dialog);

	return;
}

void enterButtonCallback(GtkWidget *widget, gpointer data){
	GdkColor	color;		//button color

	//Set the color of button
	color.red = RED_VALUE;
	color.green = GREEN_VALUE;
	color.blue = BLUE_VALUE;

	//Modify the background color of button
	gtk_widget_modify_bg(widget, GTK_STATE_PRELIGHT, &color);

	return;
}

