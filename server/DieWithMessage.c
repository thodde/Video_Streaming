/*
	File Name:		DieWithMessage.c
	Author:			Trevor Hodde
	This code is borrowed from TCP/IP Sockets in C
	By: Michael Donahoo and Kenneth Calvert
*/

#include <stdio.h>
#include <stdlib.h>

void dieWithUserMessage(const char *message, const char *detail){
	fputs(message, stderr);
	fputs(": ", stderr);
	fputs(detail, stderr);
	fputc('\n', stderr);
	exit(1);
}

void dieWithSystemMessage(const char *message){
	perror(message);
	exit(1);
}
