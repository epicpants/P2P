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
#include "tracker_parser.h"
#include "communication.h"
#include "md5.h"
using namespace std;

#define SERVER_PORT 7777
#define PEER_COMMAND_PORT 8888
#define PEER_DATA_PORT 9999
#define MAX_THREAD_COUNT 1000

int peerSocket;
pthread_t threads[MAX_THREAD_COUNT];
unsigned int threadCount = 0;
TrackerFile tf1;
//vector <


bool createTracker(string fileName)
{
  //check if tracker already exists locally
  //check if file exists locally
  //make tracker file
  //update tracker server
  string createTrackerServerCommand;
  createTrackerServerCommand = tf1.createCommand(fileName.c_str(), PEER_DATA_PORT, "Boring Description");
  cout<<createTrackerServerCommand<<endl;
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
      cout<<"Enter file name to create tracker"<<endl;
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
      cout<<"Enter name of tracker file for the file to download"<<endl;
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

  struct sockaddr_in server_addr = { AF_INET, htons( PEER_COMMAND_PORT ) };
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