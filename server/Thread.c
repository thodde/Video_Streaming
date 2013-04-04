/**
 * File Name: Thread.c
 * Author:    Trevor Hodde
 * This code was borrowed from PThreads Programming
 * By Bradford Nichols
 */

#include <pthread.h>
#include "Server.h"

void *threadMain(void *thread_arguments){
	int client_socket;
	
	//Make sure the thread is killed when it completes
	pthread_detach(pthread_self());
	//Pass args to the socket
	client_socket = ((THREAD_ARGUMENTS*)thread_arguments)->client_socket;
	//Deallocate argument
	free(thread_arguments);
	//Handle the request
	handleClientRequest(client_socket);
	
	return (NULL);
}
