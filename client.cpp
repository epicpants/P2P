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
using namespace std;

#define SERVER_PORT 7777
#define PEER_PORT 8888

int peerSocket;

bool createTracker(string fileName)
{
  //check if tracker already exists
  //check if file exists
  //make tracker
  //update tracker server
  char fileNameNTCA[fileName.length()];
  fileName.copy(fileNameNTCA, fileName.length(), 0);
  //TracerFile.create(fileNameNTCA);
  return true;
}

bool updateTracker()
{
  //push updated tracker to server
  //download new tracker
  return true;
}

bool getList()
{
  //get list of trackers from server
  return true;
}

bool getTracker()
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
  while(1)
  {
    getline(cin, userCommandString);
    stringstream userCommandStringStream(userCommandString);
    userCommandStringStream >> userCommand;
    if(userCommand == "createtracker" || userCommand == "CREATETRACKER")
    {
      string trackerFileName;
      userCommandStringStream >> trackerFileName;
      createTracker(trackerFileName);
    }
    else if (userCommand == "list" || userCommand == "LIST")
    {
      getList();
    }
    else if (userCommand == "get" || userCommand == "GET")
    {
      getTracker();
    }
    else
    {

    }
  }

}

void *peerCommandExecute(void *threadid)
{
  //read in command
  //execute command 
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
    pthread_t peerThread;
    pthread_create(&peerThread, NULL, peerCommandExecute, &temp);
  }
}

void *getFromPeer(void *threadid)
{
  //open socket
  //make request to peer for data
  //recieve data
  //close
}


int main(int argc, char* argv[])
{
  pthread_t threads[65535];
  unsigned int threadCount = 0;
  int userinputid = pthread_create(&threads[threadCount], NULL, userInput, NULL);
  threadCount++;
  int peerinputid = pthread_create(&threads[threadCount], NULL, peerInput, NULL);
  threadCount++;

  while(1)
  {

  }

  return 0;
}