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

bool createTracker(string fileName);
bool updateTracker(string fileName);
bool getList();
bool getTracker(string getTrackerFileName);


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

void *commandLineInput(void *arg)
{
	cout<<"P2P Client CLI Started"<<endl;
	string userCommandInputString;
	string userCommand;

	string trackerFileName;
	while(1)
  	{
  		getline(cin, userCommandInputString);
    	stringstream userCommandStringStream(userCommandInputString);
    	userCommandStringStream >> userCommand;
    	if(userCommand == "createtracker" || userCommand == "CREATETRACKER")
    	{
    		cout<<"createtracker"<<endl;
    		userCommandStringStream >> trackerFileName;
			createTracker(trackerFileName);
    	}
    	else if (userCommand == "list" || userCommand == "LIST")
    	{
      		cout<<"Getting list of trackers from server"<<endl;
      		getList();
    	}
    	else if (userCommand == "get" || userCommand == "GET")
    	{
      		cout<<"GET"<<endl;
      		userCommandStringStream >> trackerFileName;
      		getTracker(trackerFileName);
    	}
    	else if (userCommand == "updatetracker" || userCommand == "UPDATETRACKER")
    	{
      		cout<<"updatetracker"<<endl;
      		userCommandStringStream >> trackerFileName;
      		updateTracker(trackerFileName);
    	}
    	else
    	{
      		cout<<"Command not recognized"<<endl;
    	}
  	}	
}

int main(int argc, char* argv[])
{
 	//checks to see if the correct number of arguments were passed in
 	if(argc != 2)
	{
		cout<<"Usage: "<<argv[0]<<" ip address"<<endl;
		exit(1);
	}

	while(1)
		{

		}
}

bool createTracker(string fileName)
{
	string createTrackerServerCommand;
	createTrackerServerCommand = TrackerFile::createCommand(fileName.c_str(), PEER_PORT, "Boring Description");
	tf1.create(createTrackerServerCommand.c_str());
	int useroutputid = pthread_create(&threads[threadCount], NULL, serverinput, &serverSocket);//data from server
 	threadCount++;
  	sendServerCommand((void*)createTrackerServerCommand.c_str());
  	return true;
}

bool updateTracker(string fileName)
{
	updateTrackerServerCommand;
	updateTrackerServerCommand = TrackerFile::updateCommand(fileName.c_str(), PEER_PORT);
  	int useroutputid = pthread_create(&threads[threadCount], NULL, serverinput, NULL);//data from server
  	threadCount++;
  	sendServerCommand((void*)updateTrackerServerCommand.c_str());
  	return true;
}

bool getList()
{
	//open connection to server
  	//transmit "REQ LIST"
  	//close connection
  	int useroutputid = pthread_create(&threads[threadCount], NULL, serverinput, NULL);//data from server
  	threadCount++;
  	sendServerCommand((void*)"REQ LIST");
  	return true;
}

bool getTracker(string getTrackerFileName)
{
	//download tracker from server
  	//start new thread(getfrompeer) to download file from peers
  	getFileData getTrackerData;
  	getTrackerData.getFileDataFileName = getTrackerFileName;
  	string getTrackerServerCommand = "GET " + getTrackerFileName;
  	int useroutputid = pthread_create(&threads[threadCount], NULL, serverinput, NULL);//data from server
  	threadCount++;
  	//int peerinputid = pthread_create(&threads[threadCount], NULL, peerInput, NULL);//commands & data from peers
  	//threadCount++;
  	sendServerCommand((void*)getTrackerServerCommand.c_str());
  	return true;
}