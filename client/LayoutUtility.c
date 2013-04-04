/*
	File Name:		LayoutUtility.c
	Author:			Trevor Hodde
*/

#include "Client.h"

static GtkItemFactoryEntry		menu_items[] = {							//item factory entry array
	{"/_Media",					NULL,			NULL,					0,			"<Branch>"								},
	{"/_Help",					NULL,			NULL,					0,			"<LastBranch>"							}
};
static gint		nmenu_items = sizeof(menu_items) / sizeof(menu_items[0]);	//number of item factory entry
static GtkItemFactory	*item_factory;										//item factory widget

void initClientLayout(GtkWidget *window){
	GtkWidget	*menubar;			//menubar widget
	GtkWidget	*main_vbox;			//main vertical box widget	
	GtkWidget	*button_hbox;		//buttons horizon box widget
	GtkWidget	*separator;			//separator widget

	//Create a vbox to put menubar, image, and 4 buttons
	main_vbox = gtk_vbox_new(FALSE, BOX_SPACING_SIZE);
	//Set the border width of man_vbox
	gtk_container_set_border_width(GTK_CONTAINER(main_vbox), BORDER_WIDTH_SIZE);
	//Add the main_vbox into the window
	gtk_container_add(GTK_CONTAINER(window), main_vbox);

	//Create the menubar from itemfactoryentry
	menubar = getMenubarMenu(window);
	//Pack the menubar into the main_vbox
	gtk_box_pack_start(GTK_BOX(main_vbox), menubar, FALSE, TRUE, 0);

	//Create the toolbar
	toolbar = getToolbar();
	//Pack the toolbar into the main_vbox
	gtk_box_pack_start(GTK_BOX(main_vbox), toolbar, FALSE, TRUE, 0);

	//Create a image to play the video
	image = gtk_image_new_from_file(VIDEO_ICON);
	//Pack the image into the main_vbox
	gtk_box_pack_start(GTK_BOX(main_vbox), image, TRUE, TRUE, 0);

	//Create a separator
	separator = gtk_hseparator_new();
	//Pack the separator into the main_vbox
	gtk_box_pack_start(GTK_BOX(main_vbox), separator, FALSE, TRUE, 0);

	//Create a hbox to put 4 buttons
	button_hbox = gtk_hbox_new(TRUE, BOX_SPACING_SIZE);
	//Pack the button_hbox into the main_vbox
	gtk_box_pack_start(GTK_BOX(main_vbox), button_hbox, FALSE, TRUE, 0);

	//Create the setup button
	setupButton = getButton(button_hbox, GTK_STOCK_NETWORK, "Setup", "Setup RTSP Connection");
	//Create the play button
	playButton = getButton(button_hbox, GTK_STOCK_MEDIA_PLAY, "Play", "Play Streaming Video");
	//Create the teardown button
	teardownButton = getButton(button_hbox, GTK_STOCK_MEDIA_STOP, "Teardown", "Teardown RTSP Connection");

	//Set the sensitive property of buttons
	setSensitive(TRUE, FALSE, FALSE);

	return;
}



/*Generate Menubar Widget Function
  Variable Definition:
  -- window: gtk+ window widget pointer
  Return Value: menubar widget
*/
GtkWidget *getMenubarMenu(GtkWidget *window){
	GtkAccelGroup	*accel_group;		//accelerator group widget

	//Make an accelerator group (shortcut keys)
	accel_group = gtk_accel_group_new();
	//Make an ItemFactory (that makes a menubar)
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);
	//Generate the menu item
	gtk_item_factory_create_items(item_factory, nmenu_items, menu_items, NULL);
	//Attach the new accelerator group to the window
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

	//Return the actual menu bar created by item factory
	return gtk_item_factory_get_widget(item_factory, "<main>");
}

GtkWidget *getToolbar(){
	GtkToolItem		*tool_item;		//tool item widget
	GtkTooltips		*tooltip;		//tooltip widget

	//Create the toolbar
	toolbar = gtk_toolbar_new();
	//Set the style of toolbar
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	//Set the border width of toolbar
	gtk_container_set_border_width(GTK_CONTAINER(toolbar), BORDER_WIDTH_SIZE);

	//Create the tooltip
	tooltip = gtk_tooltips_new();

	//Create the setup tool item
	tool_item = gtk_tool_button_new_from_stock(GTK_STOCK_NETWORK);
	//Set the tooltip on the tool item
	gtk_tooltips_set_tip(tooltip, GTK_WIDGET(tool_item), "Setup", NULL);
	//Insert the tool item into toolbar
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_item, -1);
	//Create the play tool item
	tool_item = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
	//Set the tooltip on the tool item
	gtk_tooltips_set_tip(tooltip, GTK_WIDGET(tool_item), "Play", NULL);
	//Insert the tool item into toolbar
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_item, -1);
	//Create the teardown tool item
	tool_item = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
	//Set the tooltip on the tool item
	gtk_tooltips_set_tip(tooltip, GTK_WIDGET(tool_item), "Teardown", NULL);
	//Insert the tool item into toolbar
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_item, -1);
	//Create the separator tool item
	tool_item = gtk_separator_tool_item_new();
	//Insert the tool item into toolbar
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_item, -1);
	//Create the quit tool item
	tool_item = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
	//Set the tooltip on the tool item
	gtk_tooltips_set_tip(tooltip, GTK_WIDGET(tool_item), "Quit", NULL);
	//Insert the tool item into toolbar
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_item, -1);

	return toolbar;
}


GtkWidget *getButton(	GtkWidget *box,
						const gchar *icon_name,
						const gchar *label_string,
						const gchar *tooltip_label){
	GtkWidget	*icon_label_box;	//image & label box widget
	GtkWidget	*button;			//button widget
	GtkWidget	*icon;				//image widget
	GtkWidget	*label;				//label widget
	GtkTooltips	*tooltip;			//tooltip widget

	//Create the button
	button = gtk_button_new();
	//Set the button size
	gtk_widget_set_size_request(button, BUTTON_LENGTH_SIZE, BUTTON_WIDTH_SIZE);

	//Create the hbox to hold image and label
	icon_label_box = gtk_hbox_new(FALSE, BOX_SPACING_SIZE);
	//Set the border width of ilbox
	gtk_container_set_border_width(GTK_CONTAINER(icon_label_box), BORDER_WIDTH_SIZE);

	//Create the icon
	icon = gtk_image_new_from_stock(icon_name, GTK_ICON_SIZE_DND);
	//Create the label for the button
	label = gtk_label_new(label_string);

	//Pack the icon and label into the icon_label_box
	gtk_box_pack_start(GTK_BOX(icon_label_box), icon, FALSE, FALSE, BOX_PADDING_SIZE);
	gtk_box_pack_start(GTK_BOX(icon_label_box), label, FALSE, FALSE, BOX_PADDING_SIZE);

	//Create the tooltip
	tooltip = gtk_tooltips_new();
	//Set the tooltip on the button
	gtk_tooltips_set_tip(tooltip, button, tooltip_label, NULL);

	//Add the icon_label_box into the button
	gtk_container_add(GTK_CONTAINER(button), icon_label_box);
	//Pack the button into box
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

	return button;
}

/*Set the Buttons' Sensitive Property Function
  Variable Definition:
  -- setup_s: setup button sensitive
  -- play_s: play button sensitive
  -- teardown_s: teardown button sensitive
  Return Value: NULL
*/
void setSensitive(	gboolean setup_s,
					gboolean play_s,
					gboolean teardown_s){
	GtkToolItem		*tool_item;		//tool item widget

	//Set the sensitive property of buttons
	gtk_widget_set_sensitive(setupButton, setup_s);
	gtk_widget_set_sensitive(playButton, play_s);
	gtk_widget_set_sensitive(teardownButton, teardown_s);
	//Set the sensitive property of tool items
	tool_item = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), SETUP);
	gtk_widget_set_sensitive(GTK_WIDGET(tool_item), setup_s);
	tool_item = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), PLAY);
	gtk_widget_set_sensitive(GTK_WIDGET(tool_item), play_s);
	tool_item = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), TEARDOWN);
	gtk_widget_set_sensitive(GTK_WIDGET(tool_item), teardown_s);

	return;
}

/*Set the Image Function
  Variable Definition:
  -- rtp: image buffer
  Return Value: NULL
*/
void setImage(const u_int8 *rtp){
	FILE	*screen_channel;		//screenshot file stream

	//Open screenshot file stream
	screen_channel = fopen(SCREEN_NAME, "w");
	if (screen_channel == NULL){
		perror("fopen() failed");
	}
	//Write the image buffer in the file
	fwrite(rtp + RTP_HEAD_SIZE, FRAME_SIZE - RTP_HEAD_SIZE, ONE_SIZE, screen_channel);
	//Close the file stream
	fclose(screen_channel);

	//Set the image
	gtk_image_set_from_file(GTK_IMAGE(image), SCREEN_NAME);

	return;
}

/*Generate Icon Function
  Variable Definition:
  -- icon_name: icon file name
  Return Value: GdkPixBuf widget
*/
GdkPixbuf *getIcon(const gchar *icon_name){
	GdkPixbuf	*pixbuf;				//gdkpixbuf widget
	GError		*error = NULL;			//error variable

	//Create the GdkPixbuf
	pixbuf = gdk_pixbuf_new_from_file(icon_name, &error);
	//Test the pixbuf
	if (!pixbuf){
		//Output the error message
		fputs("gdk_pixbuf_new_from_file() failed: ", stderr);
		fputs(error->message, stderr);
		fputc('\n', stderr);
		//Free the error
		g_error_free(error);
	}

	return pixbuf;
}
