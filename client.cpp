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

struct getFileData
{
  string getFileDataFileName;
};

struct sendFileData
{
  string sendFileDataFileName;
  unsigned long sendFileDataPieceNumber;
  unsigned int sendFileDataPortNumber;
  string sendFileDataIPAddress;
};

void getTrackerFiles(vector<string> & tracker_list_out);

int peerSocket;
int serverSocket;
pthread_t threads[MAX_THREAD_COUNT];
unsigned int threadCount = 0;
TrackerFile tf1;
vector<string> tracker_files;
struct hostent *hostServer;
struct sockaddr_in server_addr; // = { AF_INET, htons( SERVER_PORT ) };

void *peerCommandExecute(void *threadid)
{
  //read in command
  //execute command(aka send piece)
  unsigned long numberOfPieces=0;
  pthread_exit(NULL);
}

void getTrackerFiles(vector<string> & tracker_list_out)
{
  // Open current directory
  DIR* current_dir = opendir(".");
  if(current_dir == NULL) // An error occurred
  {
    cerr<<"Error. Failed to find current directory"<<endl;
  }
  else // No errors
  {
    // Get first file
    dirent * nextEntry = readdir(current_dir);
    // While there are more files in the directory
    while(nextEntry != NULL)
    {
      string filename = nextEntry -> d_name;
      // Find if file ends in .track
      size_t tracker_found = filename.find(".track");
      if(tracker_found != string::npos)
      {
        // Add filename to list of tracker files
        tracker_list_out.push_back(filename);
      }
      nextEntry = readdir(current_dir);
    }
  }
}

bool createTracker(string fileName)
{
  //check if tracker already exists locally
  //check if file exists locally
  //make tracker file
  //update tracker server
  string createTrackerServerCommand;
  createTrackerServerCommand = TrackerFile::createCommand(fileName.c_str(), PEER_PORT, "Boring Description");
  tf1.create(createTrackerServerCommand.c_str());
  //transmit createTrackerServerCommand to server
  return true;
}

bool updateTracker(string fileName)
{
  string updateTrackerServerComamand;
  updateTrackerServerComamand = TrackerFile::updateCommand(fileName.c_str(), PEER_PORT);
  //transmit updateTrackerServerComamand
  return true;
}

bool getList()
{
  //get list of trackers from server
  //open connection to server
  //transmit "REQ LIST"
  return true;
}

bool getTracker(string getTrackerFileName)
{
  //download tracker from server
  //start new thread(getfrompeer) to download file from peers
  getFileData getTrackerData;
  getTrackerData.getFileDataFileName = getTrackerFileName;
  int getTrackerID = pthread_create(&threads[threadCount], NULL, peerCommandExecute, NULL);
  threadCount++;
  return true;
}

void *userinput(void *threadid)
{
  cout<<"P2P Client Started"<<endl;
  string userCommandString;
  string userCommand;

  string userInputBuffer;
  string userOutputBuffer;
  string trackerFileName;
  while(1)
  {
    getline(cin, userCommandString);
    stringstream userCommandStringStream(userCommandString);
    userCommandStringStream >> userCommand;
    if(userCommand == "createtracker" || userCommand == "CREATETRACKER")
    {
      cout<<"createtracker"<<endl;
      cout<<"Enter file name to create tracker"<<endl;
      userCommandStringStream >> trackerFileName;
      createTracker(trackerFileName);
    }
    else if (userCommand == "list" || userCommand == "LIST")
    {
      cout<<"list"<<endl;
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

    }
  }

}

void *peerInput(void *threadid)
{
  //open socket & listen for data

  struct sockaddr_in server_addr = { AF_INET, htons( PEER_PORT ) };
  struct sockaddr_in client_addr = { AF_INET };
  unsigned int client_len = sizeof( client_addr );
  string peerInputBuffer;
  string peerOutputBuffer;
  
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

void *serverinput(void *threadid)
{
  cout<<"USER OUTPUT THREAD!"<<endl;
  pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    //checks to see if the correct number of arguments were passed in
  if(argc != 2)
  {
    cout<<"Usage: "<<argv[0]<<" ip address"<<endl;
    exit(1);
  }

  //checks to see if the hostname is valid
  if( (hostServer = gethostbyaddr(argv[1], 4, AF_INET) ) == NULL)
  {
    cout<<"server "<<argv[1]<<" not found"<<endl;
    exit(1);
  }
  memcpy( hostServer->h_addr_list[0], (char*)&server_addr.sin_addr, hostServer->h_length );
  //creating a socket for the client
  if( (serverSocket=socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    cout<<"Socket Creation Failure"<<endl;
    exit(1);
  }
  
  //client connecting to a socket
  if( (connect( serverSocket,(struct sockaddr*)&server_addr, sizeof(server_addr))) == -1)
  {
    cout<<"Connection Failed"<<endl;
    exit(1);
  }

  getTrackerFiles(tracker_files);
  int useroutputid = pthread_create(&threads[threadCount], NULL, serverinput, NULL);//data from server
  threadCount++;
  int serverinputid = pthread_create(&threads[threadCount], NULL, userinput, NULL);//commands from stdin
  threadCount++;
  int peerinputid = pthread_create(&threads[threadCount], NULL, peerInput, NULL);//commands & data from peers
  threadCount++;

  while(1)
  {
    //Allows the two main threads to do the real work
  }

  return 0;
}