#include <iostream>
#include <sstream>
#include <pthread.h> 
#include <signal.h>
#include <cstdlib>
#include <stdio.h>
#include <cstring>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>  /* define socket */
#include <netinet/in.h>  /* define internet socket */
#include <netdb.h>       /* define internet socket */
#include <sys/stat.h>
#include <dirent.h>
#include "tracker_parser.h"
#include "communication.h"
#include "md5.h"
using namespace std;

#define SERVER_PORT 7777
#define PEER_PORT 8888
#define MAX_THREAD_COUNT 1000
#define PIECE_SIZE 1024

//create thread for handling simultaneous input & output
pthread_t gatherThread;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

//pthread_create(&gatherThread, NULL, getSegment, &arguments);

int writeOut( threadArgs* arguments, char* buffer) {
	
	
}


class p2pJob {
	
	public p2pJob( string fileName );
	
	
	
};



struct threadArgs {
	int socket;
	string filename;
	unsigned short segNum;
	unsigned long finalSeg;
	unsigned short finalSize;
};


//input from server
void getSegment(void* arg)
{
	const unsigned short bufferSize = 1024;
	unsigned short errorCode = 2;
	
	// cast void arg as threadArgs struct
	threadArgs arguments = *(threadArgs*) arg;
	
	// data buffer
	char buffer[bufferSize];

	// read in data from socket
	int bytesRead = read(arguments.socket, buffer, sizeof(buffer));
	
	// determine if byte count is valid
	if( ( bytesRead < bufferSize ) && ( arguments.segNum != arguments.finalSeg ) ) {
	
		cerr << "Invalid segment size received, exiting thread!" << endl;
		pthread_exit( errorCode );
	}
	
	// call writeOut
	pthread_mutex_lock(&mutex);
	
	//call write to file function
	//writeOut( &arguments, buffer);
	
	pthread_mutex_unlock(&mutex);
	
	close(arguments.socket);
	
	pthread_exit(0);
}
