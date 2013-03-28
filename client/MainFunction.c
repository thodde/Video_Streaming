///////////////////////////////////////////////
/*
	File Name:		MainFunction.c
	Author:			Trevor Hodde
	Note:			This MainFunction.c file
					includes Main Function.
*/
///////////////////////////////////////////////

///////////////HEADER FILES///////////////
#include "Client.h"

///////////////FUNCTIONS///////////////
/*Main Function
  Variable Definition:
  -- argc: the number of command arguments
  -- argv[]: each vairable of command arguments(argv[0] is the path of execution file forever)
  Return Value: client exit number
*/
int main(int argc, char *argv[]){
	//Test for correct number of arguments
	if (argc != 4){
		dieWithUserMessage("Parameter(s)", "<Server IP address/Name> <Server port/service> <Video file name>");
	}

	CLIENT_DATA		*cdata;			//_client_data structure node

	//Initialize GTK application
	gtk_init(&argc, &argv);

	//Allocate memory for _client_data structure
	cdata = g_malloc(sizeof(CLIENT_DATA));
	//Initialize _client_data structure
	cdata->host = argv[1];			//server ip address/name
	cdata->service = argv[2];		//server port/service
	cdata->video = argv[3];			//request video name

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

	//Show everything in window
	gtk_widget_show_all(cdata->window);
	//Hide toolbar
	gtk_widget_hide(toolbar);

	//GTK application main function
	gtk_main();
	
	return 0;
}
