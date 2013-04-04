/**
 * File Name: MainFunction.c
 * Author:    Trevor Hodde
 * This file creates a server that listens on a TCP
 * Socket for client connections. When a client connects,
 * a new thread is spawned to handled the connection to
 * keep the main socket available.
 */

#include <pthread.h>
#include <signal.h>
#include "Server.h"

int main(int argc, char *argv[]){
	if (argc != 2) {
		perror("Usage: ./VideoServer <Port>");
	}

	struct sigaction handler;
	char *service = argv[1];	//server listening port number
	int server_socket;		//socket descriptor for server
	int client_socket;		//socket descriptor for client
	int return_value;		//return value

	//Initialize the Video Server
	initServer(SERVER_RTP_PORT);
	//Set signal handler for alarm signal
	handler.sa_handler = catchAlarm;
	//Blocking everything in handler
	if (sigfillset(&handler.sa_mask) < 0) {
		perror("sigfillset() failed");
	}

	handler.sa_flags = SA_RESTART;
	//Set the "SIGALRM" signal
	if (sigaction(SIGALRM, &handler, 0) < 0) {
		perror("sigaction() failed for SIGALRM");
	}

	//Create socket for incoming connections
	server_socket = setupServerTCPSocket(service);
	if (server_socket < 0) {
		perror("setupServerTCPSocket() failed");
	}
	
	//Keep running and listening for client connections
	while(1) {
		//New connection creates a connected client socket
		client_socket = acceptTCPConnection(server_socket);
		
		//Create seperate memory for client argument
		THREAD_ARGUMENTS *thread_args = (THREAD_ARGUMENTS*)malloc(sizeof(THREAD_ARGUMENTS));
		if (thread_args == NULL) {
			perror("cannot malloc memory for_thread_arguments structure!");
		}

		//Pass the arguments to client socket
		thread_args->client_socket = client_socket;
		
		//Spawn a new client thread
		pthread_t thread_id;
		return_value = pthread_create(&thread_id, NULL, threadMain, thread_args);

		if (return_value != 0) {
			perror("pthread_create() failed");
		}

		//Output the thread id
		printf("with thread %lu\n", (unsigned long int)thread_id);
	}
	
	return 0;
}
