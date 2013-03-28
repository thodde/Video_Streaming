////////////////////////////////////////////////////////////////
/*
	File Name:		CallbackUtility.c
	Author:			Trevor Hodde
	Note:			This CallbackUtility.c file includes
					Handle GTK Widget Signal Callback Function.
*/
////////////////////////////////////////////////////////////////

///////////////HEADER FILES///////////////
#include "Client.h"

///////////////GLOBAL VARIABLE///////////////
static CLIENT_DATA	*client;		//_client_data structure node

///////////////FUNCTIONS///////////////
/*Initialize Widget Callback Function
  Variable Definition:
  -- client_data: _client_data structure node
  Return Value: NULL
*/
void initCallback(CLIENT_DATA *client_data){
	GtkToolItem		*tool_item;		//tool item widget

	//Set a handler for delete_event and destroy that exits GTK
	g_signal_connect(client_data->window, "delete-event", G_CALLBACK(deleteEventCallback), NULL);
	g_signal_connect(client_data->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	//Set a handler for enter that change the button's background color
	g_signal_connect(setupButton, "enter", G_CALLBACK(enterButtonCallback), NULL);
	g_signal_connect(playButton, "enter", G_CALLBACK(enterButtonCallback), NULL);
	g_signal_connect(pauseButton, "enter", G_CALLBACK(enterButtonCallback), NULL);
	g_signal_connect(teardownButton, "enter", G_CALLBACK(enterButtonCallback), NULL);
	//Set a handler for clicked that handle RTSP request & response message
	g_signal_connect(setupButton, "clicked", G_CALLBACK(setupRTSPCallback), client_data);
	g_signal_connect(playButton, "clicked", G_CALLBACK(playRTSPCallback), client_data);
	g_signal_connect(pauseButton, "clicked", G_CALLBACK(pauseRTSPCallback), client_data);
	g_signal_connect(teardownButton, "clicked", G_CALLBACK(teardownRTSPCallback), client_data);
	//Set a handler for clicked that handle RTSP request & response message
	tool_item = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), SETUP);
	g_signal_connect(tool_item, "clicked", G_CALLBACK(setupRTSPCallback), client_data);
	tool_item = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), PLAY);
	g_signal_connect(tool_item, "clicked", G_CALLBACK(playRTSPCallback), client_data);
	tool_item = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), PAUSE);
	g_signal_connect(tool_item, "clicked", G_CALLBACK(pauseRTSPCallback), client_data);
	tool_item = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), TEARDOWN);
	g_signal_connect(tool_item, "clicked", G_CALLBACK(teardownRTSPCallback), client_data);
	//Set a handler for delete_event that exits GTK
	tool_item = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), 5);
	g_signal_connect_swapped(tool_item, "clicked", G_CALLBACK(deleteEventCallback), client_data->window);

	//Set the global variable
	client = client_data;

	return;
}

/*Menu Callback Function
  Variable Definition:
  -- callback_data: callback function data
  -- callback_action: callback function action code
  -- menu_item: callback function widget
  Return Value: NULL
*/
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
		case PAUSE:
			//PAUSE callback function
			pauseRTSPCallback(menu_item, client);
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
					"User-Agent: Data Communications & Networks: RTSP client by %s (Unix)%s%s",	client_data->host,
																								client_data->service,
																								client_data->video, CRLF,
																								cseq_number, CRLF,
																								PROTOCOL_TYPE, PROTOCOL_METHOD,
																								client_rtp_port, CRLF,
																								MY_NAME, CRLF, CRLF);

#ifdef	DEBUG
	DEBUG_START;
	fputs("SETUP request message:\n", stdout);
	fputs(buffer, stdout);
	DEBUG_END;
#endif

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
		setSensitive(FALSE, TRUE, FALSE, TRUE);
		//Show the setup complete window
		showInfoCallback(client_data->window, "Setup complete!");
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
					"User-Agent: Data Communications & Networks: RTSP client by %s (Unix)%s%s",	client_data->host,
																								client_data->service,
																								client_data->video, CRLF,
																								cseq_number, CRLF,
																								session_id, CRLF,
																								MY_NAME, CRLF, CRLF);

#ifdef	DEBUG
	DEBUG_START;
	fputs("PLAY request message:\n", stdout);
	fputs(buffer, stdout);
	DEBUG_END;
#endif

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
		setSensitive(FALSE, FALSE, TRUE, TRUE);
	}
	//Close client socket
	close(client_socket);

	//Start RTP progress
	startRTPProgress(client_data);

	return;
}

/*PAUSE method Callback Function
  Variable Definition:
  -- widget: callback function widget
  -- client_data: callback function widget
  Return Value: NULL
*/
void pauseRTSPCallback(GtkWidget *widget, CLIENT_DATA *client_data){
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
	//Constuct RTSP PAUSE request message
	sprintf(buffer, "PAUSE rtsp://%s:%s/%s RTSP/1.0%s"	\
					"CSeq: %u%s"	\
					"Session: %u%s"	\
					"User-Agent: Data Communications & Networks: RTSP client by %s (Unix)%s%s",	client_data->host,
																								client_data->service,
																								client_data->video, CRLF,
																								cseq_number, CRLF,
																								session_id, CRLF,
																								MY_NAME, CRLF, CRLF);

#ifdef	DEBUG
	DEBUG_START;
	fputs("PAUSE request message:\n", stdout);
	fputs(buffer, stdout);
	DEBUG_END;
#endif

	//Set the buffer length
	message_length = strlen(buffer);
	//Send PAUSE request message to the server
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
		status = READY;
		//Set the buttons', toolbars' and menus' sensitive property
		setSensitive(FALSE, TRUE, FALSE, TRUE);
	}
	//Close client socket
	close(client_socket);

	//Stop RTP progress
	stopRTPProgress(status);

	return;
}

/*TEARDOWN method Callback Function
  Variable Definition:
  -- widget: callback function widget
  -- client_data: callback function widget
  Return Value: NULL
*/
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
					"User-Agent: Data Communications & Networks: RTSP client by %s (Unix)%s%s",	client_data->host,
																								client_data->service,
																								client_data->video, CRLF,
																								cseq_number, CRLF,
																								session_id, CRLF,
																								MY_NAME, CRLF, CRLF);

#ifdef	DEBUG
	DEBUG_START;
	fputs("TEARDOWN request message:\n", stdout);
	fputs(buffer, stdout);
	DEBUG_END;
#endif

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
		setSensitive(TRUE, FALSE, FALSE, FALSE);
	}
	//Close client socket
	close(client_socket);

	//Stop RTP progress
	stopRTPProgress(status);

	return;
}

/*Show or Hide Toolbar Callback Function
  Variable Definition:
  -- callback_data: callback function data
  -- callback_action: callback function action
  -- menu_item: callback function widget
  Return Value: NULL
*/
void showToolbar(gpointer callback_data, guint callback_action, GtkWidget *menu_item){
	//Test the check menu item status
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu_item))){
		//Show the toolbar
		gtk_widget_show(toolbar);
	}
	else{
		//Hide the toolbar
		gtk_widget_hide(toolbar);
	}

	return;
}

/*Show About Callback Function
  Variable Definition:
  -- widget: callback function widget
  -- data: callback function data
  Return Value: NULL
*/
void showAboutCallback(GtkWidget *widget, gpointer data){
	GtkWidget	*dialog;		//dialog widget
	GdkPixbuf	*pixbuf;		//pixbuf widget

	//Create the pixbuf
	pixbuf = getIcon(VIDEO_ICON);

	//Create the dialog
	dialog = gtk_about_dialog_new();
	//Set the name of dialog
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), WINDOW_TITLE);
	//Set the version of dialog
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "1.0");
	//Set the copyright of dialog
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "(C) Trevor Hodde");
	//Set the comment of dialog
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "Streaming Video Player is a simple media player using RTSP & RTP protocols");
	//Set the website of dialog
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "http://cs.nyu.edu/~f1226201/assignments/StreamingVideoAssignment.html");
	//Set the logo of dialog
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), pixbuf);

	//Unreference the pixbuf
	g_object_unref(pixbuf);
	//Set the pixbuf's value
	pixbuf = NULL;

	//Run the about dialog
	gtk_dialog_run(GTK_DIALOG(dialog));
	//Destroy the about dialog
	gtk_widget_destroy(dialog);

	return;
}

/*Show Information Message Dialog Callback Function
  Variable Definition:
  -- widget: callback function widget
  -- data: callback function data
  Return Value: NULL
*/
void showInfoCallback(GtkWidget *widget, gpointer data){
	GtkWidget	*dialog;		//dialog widget

	//Create a information dialog
	dialog = gtk_message_dialog_new(GTK_WINDOW(widget),
									GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_INFO,
									GTK_BUTTONS_OK,
									"%s", (gchar*)data);
	//Set the title of dialog
	gtk_window_set_title(GTK_WINDOW(dialog), "Information");
	//Run the message dialog
	gtk_dialog_run(GTK_DIALOG(dialog));
	//Destroy the message dialog
	gtk_widget_destroy(dialog);

	return;
}

/*Show Error Message Dialog Callback Function
  Variable Definition:
  -- widget: callback function widget
  -- data: callback function data
  Return Value: NULL
*/
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

/*Show Question Message Dialog Callback Function
  Variable Definition:
  -- widget: callback function widget
  -- data: callback function data
  Return Value: NULL
*/
void showQuesCallback(GtkWidget *widget, gpointer data){
	GtkWidget	*dialog;		//dialog widget

	//Create a question dialog
	dialog = gtk_message_dialog_new(GTK_WINDOW(widget),
									GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_QUESTION,
									GTK_BUTTONS_YES_NO,
									"%s", (gchar*)data);
	//Set the title of dialog
	gtk_window_set_title(GTK_WINDOW(dialog), "Question");
	//Run the message dialog
	gtk_dialog_run(GTK_DIALOG(dialog));
	//Destroy the message dialog
	gtk_widget_destroy(dialog);

	return;
}

/*Show Warning Message Dialog Callback Function
  Variable Definition:
  -- widget: callback function widget
  -- data: callback function data
  Return Value: NULL
*/
void showWarnCallback(GtkWidget *widget, gpointer data){
	GtkWidget	*dialog;		//dialog widget

	//Create a information dialog
	dialog = gtk_message_dialog_new(GTK_WINDOW(widget),
									GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_WARNING,
									GTK_BUTTONS_OK,
									"%s", (gchar*)data);
	//Set the title of dialog
	gtk_window_set_title(GTK_WINDOW(dialog), "Warning");
	//Run the message dialog
	gtk_dialog_run(GTK_DIALOG(dialog));
	//Destroy the message dialog
	gtk_widget_destroy(dialog);

	return;
}

/*Enter Button Callback Callback Function
  Variable Definition:
  -- widget: callback function widget
  -- data: callback function data
  Return Value: NULL
*/
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

/*Delete Event Callback Function
  Variable Definition:
  -- widget: callback function widget
  -- event: callback function event
  -- data: callback function data
  Return Value: NULL
*/
void deleteEventCallback(	GtkWidget *widget,
							GdkEvent *event,
							gpointer data){
	GtkWidget	*dialog;		//dialog widget
	gint		result;			//response type

	//Create a question dialog
	dialog = gtk_message_dialog_new(GTK_WINDOW(widget),
									GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_QUESTION,
									GTK_BUTTONS_YES_NO,
									"Are you sure to quit?");
	//Set the title of question dialog
	gtk_window_set_title(GTK_WINDOW(dialog), "Question");
	//Run the message dialog
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	//Test the dialog response result
	if (result == GTK_RESPONSE_NO || result == GTK_RESPONSE_DELETE_EVENT){
		//Destroy the message dialog
		gtk_widget_destroy(dialog);
	}
	else if (result == GTK_RESPONSE_YES){
		//Destroy the message dialog
		gtk_widget_destroy(dialog);
		//Exit the GTK application immediately
		gtk_main_quit();
	}

	return;
}
