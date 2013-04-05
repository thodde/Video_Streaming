/**
 * File Name: StringUtility.c
 * Author:    Trevor Hodde
 * I borrowed this entire file from:
 * yolinux.com/TUTORIALS and
 * Mastering Regular Expressions By Jeffery Friedl 
 */

#include <sys/stat.h>
#include <ctype.h>
#include <regex.h>
#include "Server.h"

char *itoa(int number){
	int i = number;	
	int	j;
	int	power;
	char	*string = (char*)malloc(sizeof(char) * (NUMBER_SIZE + 1));
	
	//Find the max power
	for (power = 1, j = 1; i >= 10; i /= 10, j++) {
		power *= 10;
	}

	//Get the string which is converted by number
	for (; power > 0; power /= 10) {
		*string++ = '0' + number / power;
		number %= power;
	}

	//Set the end of the string
	*string = '\0';

	return string - j;
}

void decodeURL(char *url){
	char	*source = NULL;	
	int	number;	

	//Initilize the source by given url
	source = url;
	while (*source) {
		if (*source == '%') {
			source++;
			//Convert HEX to number(char)
			sscanf(source, "%2x", &number);
			*url++ = number;
			//Jump the %HEX number
			source += 2;
		}
		else {
			*url++ = *source++;
		}
	}
	//Set the end of the url
	*url = '\0';
}

void pathBelowCurrentDirectory(char *url) {
	char	*source = NULL;	
	char	*destination = NULL;
	
	//Initilize the source and destination by given url
	source = destination = url;
	//Test whether the url contains "rtsp://*"
	if (strncmp(source, "rtsp://", 7) == 0) {
		//If contain "rtsp://", remove it
		source += 7;
		//Jump the server ip address/name & port number/service
		while (*source) {
			if (strncmp(source, "/", 1) == 0) {
				break;
			}
			source++;
		}
	}
	//Test whether the url contains "/../" and "//"
	while (*source) {
		//If contain "/../", remove it
		if (strncmp(source, "/../", 4) == 0) {
			source += 3;
		}
		//if contain "//", remove it
		else if (strncmp(source, "//", 2) == 0) {
			source++;
		}
		else {
			*destination++ = *source++;
		}
	}
	//Set the end of the destination string
	*destination = '\0';

	//Remove the first '/' in url
	if (*url == '/') {
		strcpy(url, url + 1);
	}

	//If the url is the root directory, set url as '.'
	if (url[0] == '\0' || strcmp(url, "./") == 0 || strcmp(url, "./..") == 0) {
		strcpy(url, ".");
	}
}

bool methodNotAllow(char *method) {
	return ((strcmp(method, "SETUP") != 0) && (strcmp(method, "PLAY") != 0)
			&& (strcmp(method, "TEARDOWN") != 0));
}

bool headerLinesIncorrect(RTSP_HEADER *header, char *field_value){
	RTSP_HEADER	*node;

	for (node = header->next; node != NULL; node = node->next) {
		//Use regular expression to check header lines' syntax
		if (!syntaxChecking(node->header_line, HEADER_LINE)) {
			sprintf(field_value, "Header line: [%s] is syntacically incorrect!", node->header_line);
			return true;
		}
	}

	return false;
}

bool urlNotExist(char *url){
	struct stat	file_information;
	
	return (stat(url, &file_information) == -1);
}

bool fieldNotExist(RTSP_HEADER *header, const char *field_name, char *field_value) {
	RTSP_HEADER	*node;
	
	//Initialize node pointer
	node = header;
	//Find the field name
	while (node != NULL) {
		if (strcmp(node->field_name, field_name) != 0) {
			node = node->next;
		}
		else {
			break;
		}
	}
	//Get the field value
	if (node != NULL) {
		strcpy(field_value, node->field_value);
	}
	
	return (node == NULL);
}

bool urlIsADirectory(char *url) {
	struct stat file_information;
	
	return (stat(url, &file_information) != -1 && S_ISDIR(file_information.st_mode));
}

bool methodIsSetup(const char *method) {
	return strcmp(method, "SETUP") == 0;
}

bool methodIsPlay(const char *method) {
	return strcmp(method, "PLAY") == 0;
}

bool methodIsTeardown(const char *method) {
	return strcmp(method, "TEARDOWN") == 0;
}

bool methodIsNotValidInState(const char *method) {
	//Test the status is INIT
	if (status == INIT) {
		//PLAY method is not valid
		return (methodIsPlay(method));
	}
	//Test the status is READY or PLAYING
	else {
		//SETUP method is not valid
		return methodIsSetup(method);
	}
}

char *splitNameAndValue(char *header_line, const char stop) {
	int	i = 0;	
	int 	j = 0;	
	char	*name = (char*)malloc(sizeof(char)*(strlen(header_line) + 1));	
	
	//Jump the ' ' or Tab character(blank character)
	for (i = 0; isspace(header_line[i]); i++) { ; }	
	
	//Set the name in the header line
	for (j = 0; header_line[i] && (header_line[i] != stop); i++, j++) {
		//Lowercase the field name
		name[j] = tolower(header_line[i]);
	}

	//Set the end of the name string
	name[j] = '\0';
	
	//Test whether has more characters after name
	if (header_line[i]) {
		//Jump the "stop" character
		++i;
		//Jump the ' ' or Tab character(blank chacater)
		for (; isspace(header_line[i]); i++){ ; }
	}
	//Set the value in the header line and remove the '\r' and '\n' character
	for (j = 0; header_line[i] && (header_line[i] != '\r') && (header_line[i] != '\n'); i++, j++){
		header_line[j] = header_line[i];
	}

	//Set the end of the value string
	header_line[j] = '\0';
	
	return name;
}

bool syntaxChecking(char *string, int signal_value){
	char 	*pattern = (char*)malloc(sizeof(char) * (BUFFER_SIZE * 2));
	char	*error_buffer = (char*)malloc(sizeof(char) * (STRING_SIZE));	
	int 		status;	
	size_t 		nmatch = ONE_SIZE;
	regex_t 	reg;		
	regmatch_t	pmatch[ONE_SIZE];	
	
	//According to the signal value, determine using which regular expression
	switch (signal_value) {
		//Blank line
		case BLANK_LINE:
			pattern = "^[\t| ]*\r\n$";
			break;
		//Request line
		case REQUEST_LINE:
			pattern = "^[\t| ]*[^\t| ]+[ ][^\t| ]+[ ][^\t| ]+\r\n$";
			break;
		//RTSP URL format
		case URL_FORMAT:
			pattern = 	"^(rtsp://(((2[0-4][0-9]|25[0-5]|[01]?[0-9][0-9]?)[.]){3}(2[0-4][0-9]|25[0-5]|[01]?[0-9][0-9]?)"	\
						"|([0-9a-z_!~*'()-]+[.])*([0-9a-z][0-9a-z-]{0,61})?[0-9a-z][.][a-z]{2,6}"	\
						"|(localhost))"	\
						"(:([0-9]|[1-9][0-9]|[1-9][0-9]{2}|[1-9][0-9]{3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5]))?)?"\
						"(/[A-Z0-9a-z_!~*'().;?:@&=+$,%#-]+)+/?$";
			break;
		//RTSP version format
		case RTSP_VERSION:
			pattern = "^RTSP/[0-9]+[.][0-9]+$";
			break;
		//RTSP version 1.0
		case RTSP_VERSION_1:
			pattern = "^RTSP/[0]*[1][.][0]*[0]$";
			break;
		//Header lines
		case HEADER_LINE:
			pattern = "^[\t| ]*[a-zA-Z-]+:(([\t| ]*)|(.+[^\t| ]))\r\n$";
			break;
		//Range format
		case RANGE_FORMAT:
			pattern = "^npt=[0-9]+-?([0-9]+)?$";
			break;
		//Time format(three kinds of format)
		case TIME_FORMAT:
			pattern = 	"^((Sun|Mon|Tue|Wed|Thu|Fri|Sat),"	\
						"[ ]((((31[ ](Jan|Mar|May|Jul|Aug|Oct|Dec))"	\
						"|((0[1-9]|[12][0-9]|30)[ ](Jan|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec))"	\
						"|((0[1-9]|1[0-9]|2[0-8])[ ]Feb))[ ]([0-9]{4}))"	\
						"|(29[ ]Feb[ ]([0-9]{2}(0[48]|[2468][048]|[13579][26])|(0[48]|[2468][048]|[13579][26])00)))"	\
						"[ ](([01][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9]))[ ]GMT)"	\
						"|((Sunday|Monday|Tuesday|Wednesday|Thursday|Friday|Saturday),"	\
						"[ ]((((31-(Jan|Mar|May|Jul|Aug|Oct|Dec))"	\
						"|((0[1-9]|[12][0-9]|30)-(Jan|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec))"	\
						"|((0[1-9]|1[0-9]|2[0-8])-Feb))-([0-9]{2}))"	\
						"|(29-Feb-(0[048]|[2468][048]|[13579][26])))"	\
						"[ ](([01][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9]))[ ]GMT)"	\
						"|((Sun|Mon|Tue|Wed|Thu|Fri|Sat)"	\
						"[ ](((((Jan|Mar|May|Jul|Aug|Oct|Dec)[ ]31)"	\
						"|((Jan|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)[ ]([ ][1-9]|[12][0-9]|30))"	\
						"|(Feb[ ]([ ][1-9]|1[0-9]|2[0-8])))"	\
						"[ ](([01][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9]))"	\
						"[ ]([0-9]{4}))"	\
						"|(Feb[ ]29[ ](([01][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9]))"	\
						"[ ]([0-9]{2}(0[48]|[2468][048]|[13579][26])|(0[48]|[2468][048]|[13579][26])00))))$";
			break;
		default:
			pattern = "";
			break;
	}
	//Compile the regular expression
	status = regcomp(&reg, pattern, REG_EXTENDED);
	if (status != 0) {
		regerror(status, &reg, error_buffer, STRING_SIZE);
		perror("regcomp() failed!");
	}

	//Match the regular expression
	status = regexec(&reg, string, nmatch, pmatch, 0);
	//If there is no match result, return 0
	if (status == REG_NOMATCH) {
		return false;
	}

	//Close the regex_t structure
	regfree(&reg);
	
	return true;
}
