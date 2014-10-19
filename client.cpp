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
#include "tracker_parser.h"
#include "communication.h"
using namespace std;

#define SERVER_PORT 7777
#define PEER_PORT 8888

int peerSocket;
pthread_t threads[65535];
unsigned int threadCount = 0;


bool createTracker(string fileName)
{
  //check if tracker already exists locally
  //check if file exists locally
  //make tracker file
  //update tracker server
  char fileNameNTCA[fileName.length()];
  fileName.copy(fileNameNTCA, fileName.length(), 0);
  //TracerFile.create(fileNameNTCA);
  return true;
}

bool updateTracker()
{
  //push updated tracker to server
  //download new trackers
  return true;
}

bool getList()
{
  //get list of trackers from server
  return true;
}

bool getTracker(string getTrackerFileName)
{
  //download tracker from server
  //start new thread(getfrompeer) to download file from peers
  return true;
}

void *userInput(void *threadid)
{
  cout<<"P2P Client Started"<<endl;
  string userCommandString;
  string userCommand;
  string trackerFileName;
  while(1)
  {
    getline(cin, userCommandString);
    stringstream userCommandStringStream(userCommandString);
    userCommandStringStream >> userCommand;
    if(userCommand == "createtracker" || userCommand == "CREATETRACKER")
    {
      userCommandStringStream >> trackerFileName;
      createTracker(trackerFileName);
    }
    else if (userCommand == "list" || userCommand == "LIST")
    {
      getList();
    }
    else if (userCommand == "get" || userCommand == "GET")
    {
      userCommandStringStream >> trackerFileName;
      getTracker(trackerFileName);
    }
    else
    {

    }
  }

}

void *peerCommandExecute(void *threadid)
{
  //read in command
  //execute command(aka send piece)
  pthread_exit(NULL);
}

void *peerInput(void *threadid)
{
  //open socket
  //listen for data
  //start peerCommandExecute Thread 

  struct sockaddr_in server_addr = { AF_INET, htons( PEER_PORT ) };
  struct sockaddr_in client_addr = { AF_INET };
  unsigned int client_len = sizeof( client_addr );
  
  /* create a stream socket */
  if( ( peerSocket = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
  {
    cerr << "Client: socket failed" << endl;
    exit( 1 );
  }
  
  /* bind the socket to an internet port */
  if( bind(peerSocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 )
  {
    cerr << "Client: bind failed" << endl;
    exit( 1 );
  }

  /* listen for clients */
  if( listen( peerSocket, 1 ) == -1 )
  {
    cerr << "Client: listen failed" << endl;
    exit( 1 );
  }

  cout << "Client is listening for Peers to establish a connection\n";

  int temp;
  while((temp = accept(peerSocket, (struct sockaddr*)&client_addr, &client_len )) > 0)
  {
    pthread_create(&threads[threadCount], NULL, peerCommandExecute, &temp);
    threadCount++;
  }
}

void *getFromPeer(void *threadid)
{
  //get list of peers
  //open socket
  //make request to peers for data
  //recieve data
  //merge data
  //close socket
  pthread_exit(NULL);
}


int main(int argc, char* argv[])
{
  int userinputid = pthread_create(&threads[threadCount], NULL, userInput, NULL);
  threadCount++;
  int peerinputid = pthread_create(&threads[threadCount], NULL, peerInput, NULL);
  threadCount++;

  while(1)
  {
    //Allows the two main threads to do the real work
  }

  return 0;
}