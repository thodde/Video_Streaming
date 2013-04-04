/**
 * File Name: MainFunction.c
 * Author:    Trevor Hodde
 * The code to setup the GTK window was borrowed from:
 * docs.gstreamer.com/display
 */

#include "Client.h"

int main(int argc, char *argv[]) {
	if (argc != 4){ 
		perror("Usage: <Server IP address/hostname> <Server port> <video file>");
	}

	CLIENT_DATA	*cdata;

	//Initialize GTK application
	gtk_init(&argc, &argv);

	//Allocate memory for _client_data structure
	cdata = g_malloc(sizeof(CLIENT_DATA));
	//Initialize _client_data structure
	cdata->host = argv[1];			//server ip address
	cdata->service = argv[2];		//server port
	cdata->video = argv[3];			//video name

	//Create the main window
	cdata->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//Set the position of window
	gtk_window_set_position(GTK_WINDOW(cdata->window), GTK_WIN_POS_CENTER);
	//Set the default size of window
	gtk_window_set_default_size(GTK_WINDOW(cdata->window), WINDOW_LENGTH_SIZE, WINDOW_WIDTH_SIZE);
	//Set the icon of window
	gtk_window_set_icon(GTK_WINDOW(cdata->window), getIcon(VIDEO_ICON));
	//Set the title of window
	gtk_window_set_title(GTK_WINDOW(cdata->window), WINDOW_TITLE);

	//Initialize the menubar, toolbar, image, and button widget
	initClientLayout(cdata->window);
	//Initialize widget callback function
	initCallback(cdata);
	//Initalize video client global variables
	initClient(CLIENT_RTP_PORT);

	gtk_widget_show_all(cdata->window);
	gtk_widget_hide(toolbar);

	//GTK application main function
	gtk_main();
	
	return 0;
}
