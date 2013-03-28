////////////////////////////////////////////////////////////
/*
	File Name:		FileUtility.c
	Author:			Trevor Hodde
	Note:			This FileUtility.c file includes 
					Handle Server File Functions.
*/
////////////////////////////////////////////////////////////

///////////////HEADER FILES///////////////
#include "Server.h"

///////////////FUNCTIONS///////////////
/*Get File Type Function
  Variable Definition:
  -- url: the request url except domain name and port number
  Return Value: pointer to the file extension type
*/
char *getFileType(char *url){
	char *extension = NULL;		//file extension
	
	//Get the file extension type by '.' character
	if ((extension = strrchr(url, '.')) != NULL){
		//Jump the '.' character
		return (extension + 1);
	}
	return "";
}
