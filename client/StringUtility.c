/*
	File Name:		StringUtility.c
	Author:			Trevor Hodde
*/

#include <ctype.h>
#include "Client.h"

char *itoa(int number){
	int		i = number;													//number
	int		j;															//counter
	int		power;														//power
	char	*string = (char*)malloc(sizeof(char) * (NUMBER_SIZE + 1));	//convert string
	
	//Find the max power
	for (power = 1, j = 1; i >= 10; i /= 10, j++){
		power *= 10;
	}
	//Get the string which is converted by number
	for (; power > 0; power /= 10){
		*string++ = '0' + number / power;
		number %= power;
	}
	//Set the end of the string
	*string = '\0';

	return string - j;
}

bool fieldExist(RTSP_HEADER *header, const char *field_name, char *field_value){
	RTSP_HEADER		*node;		//_rtsp_header structure node
	
	//Initialize node pointer
	node = header;
	//Find the field name
	while (node != NULL){
		if (strcmp(node->field_name, field_name) != 0){
			node = node->next;
		}
		else{
			break;
		}
	}
	//Get the field value
	if (node != NULL){
		strcpy(field_value, node->field_value);
	}
	
	return (node != NULL);
}

char *splitNameAndValue(char *header_line, const char stop){
	int		i = 0;															//counter for name
	int 	j = 0;															//counter for value
	char	*name = (char*)malloc(sizeof(char)*(strlen(header_line) + 1));	//name string
	
	//Jump the ' ' or Tab character(blank character)
	for (i = 0; isspace(header_line[i]); i++){
		;	
	}
	//Set the name in the header line
	for (j = 0; header_line[i] && (header_line[i] != stop); i++, j++){
		//Lowercase the field name
		name[j] = tolower(header_line[i]);
	}
	//Set the end of the name string
	name[j] = '\0';
	
	//Test whether has more characters after name
	if (header_line[i]){
		//Jump the "stop" character
		++i;
		//Jump the ' ' or Tab character(blank chacater)
		for (; isspace(header_line[i]); i++){
			;
		}
	}
	//Set the value in the header line and remove the '\r' and '\n' character
	for (j = 0; header_line[i] && (header_line[i] != '\r') && (header_line[i] != '\n'); i++, j++){
		header_line[j] = header_line[i];
	}
	//Set the end of the value string
	header_line[j] = '\0';
	
	return name;
}
