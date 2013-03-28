/**
 *	File Name:	FileUtility.c
 *	Author:		Trevor Hodde
 */

#include "Server.h"

char *getFileType(char *url){
	char *extension = NULL;
	
	//Get the file extension type using the '.' character
	if ((extension = strrchr(url, '.')) != NULL){
		//Jump the '.' character
		return (extension + 1);
	}
	return "";
}
