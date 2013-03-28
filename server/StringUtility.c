////////////////////////////////////////////////////////////
/*
	File Name:		StringUtility.c
	Instructor:		Prof. Arthur Goldberg
	Author:			Trevor Hodde
	UID:			N14361265
	Department:		Computer Science
	Note:			This StringUtility.c file includes 
					Handle String Functions.
*/
////////////////////////////////////////////////////////////

///////////////HEADER FILES///////////////
#include <sys/stat.h>
#include <ctype.h>
#include <regex.h>
#include "Server.h"

///////////////FUNCTIONS///////////////
/*Convert int type to char* type Function
  Variable Definition:
  -- number: original number
  Return Value: convert string
*/
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

/*Decode %HEX Format URL Function
  Variable Definition:
  -- url: the request url except domain name and port number
  Return Value: NULL
*/
void decodeURL(char *url){
	char	*source = NULL;		//source string
	int		number;				//number(char) represent original %HEX

	//Initilize the source by given url
	source = url;
	//Test the %HEX in url variable
	while (*source){
		if (*source == '%'){
			source++;
			//Convert HEX to number(char)
			sscanf(source, "%2x", &number);
			*url++ = number;
			//Jump the %HEX number
			source += 2;
		}
		else{
			*url++ = *source++;
		}
	}
	//Set the end of the url
	*url = '\0';
}

/*Get paths below the current directory Function
  Variable Definition:
  -- url: the request url except domain name and port number
  Return Value: NULL
*/
void pathBelowCurrentDirectory(char *url){
	char	*source = NULL;			//source string
	char	*destination = NULL;	//destination string
	
	//Initilize the source and destination by given url
	source = destination = url;
	//Test whether the url contains "rtsp://*"
	if (strncmp(source, "rtsp://", 7) == 0){
		//If contain "rtsp://", remove it
		source += 7;
		//Jump the server ip address/name & port number/service
		while (*source){
			if (strncmp(source, "/", 1) == 0){
				break;
			}
			source++;
		}
	}
	//Test whether the url contains "/../" and "//"
	while (*source){
		//If contain "/../", remove it
		if (strncmp(source, "/../", 4) == 0){
			source += 3;
		}
		//if contain "//", remove it
		else if (strncmp(source, "//", 2) == 0){
			source++;
		}
		else{
			*destination++ = *source++;
		}
	}
	//Set the end of the destination string
	*destination = '\0';
	//Remove the first '/' in url
	if (*url == '/'){
		strcpy(url, url + 1);
	}
	//If the url is the root directory, set url as '.'
	if (url[0] == '\0' || strcmp(url, "./") == 0 || strcmp(url, "./..") == 0){
		strcpy(url, ".");
	}
}

/*Check RTSP Method Function
  Variable Definition:
  -- method: the request method
  Return Value: if method is not allowed, return 1; else return 0
*/
bool methodNotAllow(char *method){
	return ((strcmp(method, "SETUP") != 0) && (strcmp(method, "PLAY") != 0)
			&& (strcmp(method, "PAUSE") != 0) && (strcmp(method, "TEARDOWN") != 0));
}

/*Check Header Lines Function
  Variable Definition:
  -- header: client header lines
  -- field_value: syntax incorrect header line string
  Return Value: if all header lines are syntactically incorrect, return 1; else return 0
*/
bool headerLinesIncorrect(RTSP_HEADER *header, char *field_value){
	RTSP_HEADER		*node;		//_rtsp_header structure node

	for (node = header->next; node != NULL; node = node->next){
		//Use regular expression to check header lines' syntax
		if (!syntaxChecking(node->header_line, HEADER_LINE)){
			sprintf(field_value, "Header line: [%s] is syntacically incorrect!", node->header_line);
			return true;
		}
	}

	return false;
}

/*Test URL is not exist Function
  Variable Definition:
  -- url: the request url except domain name and port number
  Return Value: if the url is not exist, return 0; else, return 1
*/
bool urlNotExist(char *url){
	struct stat		file_information;	//file information structure
	
	return (stat(url, &file_information) == -1);
}

/*Test Header Field is not exist Function
  Variable Definition:
  -- header: _rtsp_header structure head pointer
  -- field_name: field name
  -- field_value: field value
  Return Value: if field exist, return 0; else return 1
*/
bool fieldNotExist(RTSP_HEADER *header, const char *field_name, char *field_value){
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
	
	return (node == NULL);
}

/*Test URL is a directory Function
  Variable Definition:
  -- url: the request url except domain name and port number
  Return Value: if the url is a directory, return 1; else return 0
*/
bool urlIsADirectory(char *url){
	struct stat		file_information;	//file information structure
	
	return (stat(url, &file_information) != -1 && S_ISDIR(file_information.st_mode));
}

/*Test Method is SETUP Function
  Variable Definition:
  -- method: the request method
  Return Value: if the method is SETUP, return 1; else return 0
*/
bool methodIsSetup(const char *method){
	return strcmp(method, "SETUP") == 0;
}

/*Test Method is PLAY Function
  Variable Definition:
  -- method: the request method
  Return Value: if the method is PLAY, return 1; else return 0
*/
bool methodIsPlay(const char *method){
	return strcmp(method, "PLAY") == 0;
}

/*Test Method is PAUSE Function
  Variable Definition:
  -- method: the request method
  Return Value: if the method is PAUSE, return 1; else return 0
*/
bool methodIsPause(const char *method){
	return strcmp(method, "PAUSE") == 0;
}

/*Test Method is TEARDOWN Function
  Variable Definition:
  -- method: the request method
  Return Value: if the method is TEARDOWN, return 1; else return 0
*/
bool methodIsTeardown(const char *method){
	return strcmp(method, "TEARDOWN") == 0;
}

/*Test Method is Not Valid in State Function
  Variable Definition:
  -- method: the request method
  Return Value: if the method is not valid in special state, return 1, else return 0
*/
bool methodIsNotValidInState(const char *method){
	//Test the status is INIT
	if (status == INIT){
		//PLAY and PAUSE method is not valid
		return (methodIsPlay(method) || methodIsPause(method));
	}
	//Test the status is READY or PLAYING
	else{
		//SETUP method is not valid
		return methodIsSetup(method);
	}
}

/*Split the Name and Value of Header Line Function
  Variable Definition:
  -- header_line: client request header lines
  -- stop: split character
  Return Value: name of the header_line (before the stop of the header_line)
*/
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

/*Use Regular Expressions to check Request Message Function
  Variable Definition:
  -- string: string need to be checked
  -- signal_value: signal that decide the checking type
  Return Value: if matched, return 1; else return 0
*/
bool syntaxChecking(char *string, int signal_value){
	char 		*pattern = (char*)malloc(sizeof(char) * (BUFFER_SIZE * 2));		//regular expression string
	char		*error_buffer = (char*)malloc(sizeof(char) * (STRING_SIZE));	//error buffer
	int 		status;															//result status
	size_t 		nmatch = ONE_SIZE;												//max number of match result
	regex_t 	reg;															//regex_t structure
	regmatch_t	pmatch[ONE_SIZE];												//match result
	
	//According to the signal value, determine using which regular expression
	switch (signal_value){
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
	if (status != 0){
		regerror(status, &reg, error_buffer, STRING_SIZE);
		dieWithUserMessage("regcomp() failed!", error_buffer);
	}
	//Match the regular expression
	status = regexec(&reg, string, nmatch, pmatch, 0);
	//If there is no match result, return 0
	if (status == REG_NOMATCH){
		return false;
	}
	//Close the regex_t structure
	regfree(&reg);
	
	return true;
}
