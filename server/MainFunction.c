///////////////////////////////////////////////
/*
	File Name:		MainFunction.c
	Instructor:		Prof. Arthur Goldberg
	Author:			Trevor Hodde
	UID:			N14361265
	Department:		Computer Science
	Note:			This MainFunction.c file
					includes Main Function.
*/
///////////////////////////////////////////////

///////////////HEADER FILES///////////////
#include <pthread.h>
#include <signal.h>
#include "Server.h"

///////////////FUNCTIONS///////////////
/*Main Function
  Variable Definition:
  -- argc: the number of command arguments
  -- argv[]: each vairable of command arguments(argv[0] is the path of execution file forever)
  Return Value: Server exit number
*/
int main(int argc, char *argv[]){
	//Test for correct number of arguments
	if (argc != 2){
		dieWithUserMessage("Parameter(s)", "<Server Port/Service>");
	}

	struct sigaction	handler;	//signal handler
	char	*service = argv[1];		//server listening port number
	int 	server_socket;			//socket descriptor for server
	int 	client_socket;			//socket descriptor for client
	int		return_value;			//return value

	//Initialize the Video Server
	initServer(SERVER_RTP_PORT);
	//Set signal handler for alarm signal
	handler.sa_handler = catchAlarm;
	//Blocking everything in handler
	if (sigfillset(&handler.sa_mask) < 0){
		dieWithSystemMessage("sigfillset() failed");
	}
	/*Set SA_RESTART flags is very important,
	  accept could not be interrupted by timer*/
	handler.sa_flags = SA_RESTART;
	//Set the "SIGALRM" signal
	if (sigaction(SIGALRM, &handler, 0) < 0){
		dieWithSystemMessage("sigaction() failed for SIGALRM");
	}

	//Create socket for incoming connections
	server_socket = setupServerTCPSocket(service);
	if (server_socket < 0){
		dieWithSystemMessage("setupServerTCPSocket() failed");
	}
	
	//Run forever
	for (;;){
		//New connection creates a connected client socket
		client_socket = acceptTCPConnection(server_socket);
		
		//Create seperate memory for client argument
		THREAD_ARGUMENTS *thread_args = (THREAD_ARGUMENTS*)malloc(sizeof(THREAD_ARGUMENTS));
		if (thread_args == NULL){
			dieWithUserMessage("malloc() failed", "cannot malloc memory for _thread_arguments structure!");
		}
		//Pass the arguments to client socket
		thread_args->client_socket = client_socket;
		
		//Create client thread
		pthread_t thread_id;			//thread id number
		return_value = pthread_create(&thread_id, NULL, threadMain, thread_args);
		if (return_value != 0){
			dieWithUserMessage("pthread_create() failed", strerror(return_value));
		}
		//Output the thread id
		printf("with thread %lu\n", (unsigned long int)thread_id);
	}
	
	return 0;
}
