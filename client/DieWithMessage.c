///////////////////////////////////////////////////////////
/*
	File Name:		DieWithMessage.c
	Author:			Trevor Hodde
	Note:			This DieWithMessage.c file includes
					User Error Message Function and
					System Error Message Function.
*/
///////////////////////////////////////////////////////////

///////////////HEADER FILES///////////////
#include <stdio.h>
#include <stdlib.h>

///////////////FUNCTIONS///////////////
/*User Error Message Function
  Variable Definition:
  -- message: summary of error message
  -- detail: detail error message based on error code
  Return value: NULL
*/
void dieWithUserMessage(const char *message, const char *detail){
	fputs(message, stderr);
	fputs(": ", stderr);
	fputs(detail, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*System Error Message Function
  Variable Definition:
  -- message: summary of error message
  Return value: NULL
*/
void dieWithSystemMessage(const char *message){
	perror(message);
	exit(1);
}
