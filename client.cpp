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

void* sendServerCommand(void* cmd);
void getTrackerFiles(vector<string> & tracker_list_out);

int peerSocket;
int serverSocket;
pthread_t threads[MAX_THREAD_COUNT];
unsigned int threadCount = 0;
TrackerFile tf1;
vector<string> tracker_files;
struct hostent *hostServer;
struct hostent *hostPeer;
struct sockaddr_in server_addr = { AF_INET, htons( SERVER_PORT ) };
struct sockaddr_in peer_addr = { AF_INET, htons( PEER_PORT)};

void* sendServerCommand(void* cmd)
{
  char* cmdStr = (char*)cmd;

  stringstream ss(cmdStr);
  string trackerFileName;
  ss >> trackerFileName >> trackerFileName;

  int clientSocket;

  //creating a socket for the client
  if((clientSocket=socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    cout<<"Socket Creation Failure"<<endl;
    exit(1);
  }
  
  //client connecting to a socket
  if((connect(clientSocket,(struct sockaddr*)&server_addr, sizeof(server_addr))) == -1)
  {
    cout<<"Connection Failed"<<endl;
    exit(1);
  }

  write(clientSocket,cmdStr, PIECE_SIZE);

  char buffer[PIECE_SIZE];
  read(clientSocket,buffer,sizeof(buffer));

  if(strstr(buffer,"create"))
  {
    cout << buffer << endl;
  }
  else if(strstr(buffer,"update"))
  {
    cout << buffer << endl;
  }
  else if(strstr(buffer,"LIST"))
  {
    stringstream ss(buffer);
    string tmp,filename;
    int numFiles;
    ss >> tmp >> tmp >> numFiles;

    tracker_files.clear();

    for(int f=0; f<numFiles; f++)
    {
      read(clientSocket, buffer, sizeof(buffer));
      stringstream sbuf(buffer);
      sbuf >> tmp;
      if(tmp!="Error")
      {
        sbuf >> filename;
        tracker_files.push_back(filename+".track");
        cout << filename << ".track" << endl;
      }
    }
    read(clientSocket, buffer, sizeof(buffer));
    if(!strstr(buffer,"END"))
    {
      cerr << "LIST RESPONSE ERROR" << endl;
    }
    else
    {
      cout << "Received list of tracker files. TADAH!" << endl;
    }
  }
  else if(strstr(buffer,"GET"))
  {
    remove(trackerFileName.c_str());
    FILE* trackerFile = fopen(trackerFileName.c_str(),"w");
    string newBuf;

    if(trackerFile != NULL)
    {
      read(clientSocket, buffer, sizeof(buffer));
      do
      {
        cout << "WRITING TO FILE" << endl;
        newBuf = buffer;
        if(newBuf.size() < PIECE_SIZE)
        {
          fwrite((void*)buffer,sizeof(char),newBuf.size(),trackerFile);
        }
        else
          fwrite((void*)buffer,sizeof(char),PIECE_SIZE,trackerFile);
        read(clientSocket, buffer, sizeof(buffer));
      } while(!strstr(buffer,"REP GET END"));
      cout << "Finished download" << endl;
      fclose(trackerFile);
    }


    TrackerFile tfile;
    const char* tname = trackerFileName.c_str();
    tfile.parseTrackerFile(tname);
    const char* fname = tfile.getFilename().c_str();
    int numHosts = tfile.getNumHosts();
    long filesize = tfile.getFilesize();
    unsigned long num_chunks = ((filesize - 1) / (PIECE_SIZE)) + 1;
    HostInfo peerHost = tfile[0];
    const char* peer_ip = peerHost.ipaddr.c_str();
    int peer_port = peerHost.port;

    // Connecting to peer
    int ps;
    if( (hostPeer = gethostbyname(peer_ip) ) == NULL)
    {
      cout<<"host not found"<<endl;
      exit(1);
    }

    memcpy( hostPeer->h_addr_list[0], (char*)&peer_addr.sin_addr, hostPeer->h_length );

    //creating a socket for the client
    if((ps=socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
      cout<<"Socket Creation Failure"<<endl;
      exit(1);
    }
    
    //client connecting to a socket
    if((connect(ps,(struct sockaddr*)&peer_addr, sizeof(peer_addr))) == -1)
    {
      cout<<"Connection Failed"<<endl;
      exit(1);
    }

    strcpy(buffer, "GET ");
    strcat(buffer, fname);
    strcat(buffer, " 0");
    write(ps, buffer, sizeof(buffer));



    // NOW READ IN CHUNKS

    remove(fname);
    FILE* peerFile = fopen(fname,"wb");
    string newBuf2;
    //size_t loc2;

    if(peerFile != NULL)
    {
      read(ps, buffer, sizeof(buffer));
      do
      {
        cout << "WRITING TO FILE" << endl;
        newBuf2 = buffer;
        if(newBuf2.size() < PIECE_SIZE)
        {
          fwrite((void*)buffer,sizeof(char),newBuf2.size(),peerFile);
        }
        else
          fwrite((void*)buffer,sizeof(char),PIECE_SIZE,peerFile);
        read(ps, buffer, sizeof(buffer));
      } while(!strstr(buffer,"REP GET END"));
      cout << "Finished download" << endl;
      fclose(peerFile);
    }


  }
}

void *serverinput(void *threadid)
{
  cout<<"Server Input THREAD!"<<endl;
  pthread_exit(NULL);
}

void *runPeer(void *threadid)
{
  // Code for parsing peer input command

  if(threadid == NULL)
  {
    cerr << "Thread received null argument" << endl;
  }
  
  int skt = *(int *)threadid;
  char buffer[1024];
  char response[1024];

  stringstream ss(buffer);
  string file_name;
  FILE* file;
  long tracker_filesize;
  char* file_contents;
  ss >> file_name >> file_name;
  cout << "Client requested GET for " << file_name << endl;
  strcpy(response, "REP GET BEGIN");
  write(skt, response, sizeof(response));
  
  file = fopen(file_name.c_str(), "rb");
  if(file != NULL)
  {
    fseek(file, 0L, SEEK_END);
    tracker_filesize = ftell(file);
    rewind(file);
    long bytes_read = 0;
    file_contents = (char*) malloc(sizeof(char) * tracker_filesize);
    while(bytes_read < tracker_filesize)
    {
      if(bytes_read + PIECE_SIZE > tracker_filesize)
      {
        bytes_read += fread(response, sizeof(char), tracker_filesize % PIECE_SIZE, file);
        response[tracker_filesize % PIECE_SIZE] = '\0';
      }
      else
      {
        bytes_read += fread(response, sizeof(char), PIECE_SIZE, file);
      } 
      write(skt, response, sizeof(response));
      strcat(file_contents, response);
    }
  }
  fclose(file);
  string contents = file_contents;
  free(file_contents);
  //MD5 md5(contents);
  //const char* file_md5 = md5.hexdigest().c_str();
  strcpy(response, "REP GET END");
  //strcat(response, file_md5);
  write(skt, response, sizeof(response));

  pthread_exit(NULL);
}

void *peerInput(void *threadid)
{ 
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
  if( listen( peerSocket, 10 ) == -1 )
  {
    cerr << "Client: listen failed" << endl;
    exit( 1 );
  }

  cout << "Client is listening for clients to establish a connection\n";

  int temp;
  while((temp = accept(peerSocket, (struct sockaddr*)&client_addr, &client_len )) > 0)
  {  
  pthread_t peerThread;
  pthread_create(&peerThread, NULL, runPeer, &temp);
  }
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
  string updateTrackerServerCommand;
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
  int peerinputid = pthread_create(&threads[threadCount], NULL, peerInput, NULL);//commands & data from peers
  threadCount++;
  sendServerCommand((void*)getTrackerServerCommand.c_str());
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
      // int peerinputid = pthread_create(&threads[threadCount], NULL, peerInput, NULL);//commands & data from peers
      // threadCount++;<<"updatetracker"<<endl;
      userCommandStringStream >> trackerFileName;
      updateTracker(trackerFileName);
    }
    else
    {
      cout<<"Command not recognized"<<endl;
    }
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
  //checks to see if the correct number of arguments were passed in
  if(argc != 2)
  {
    cout<<"Usage: "<<argv[0]<<" ip address"<<endl;
    exit(1);
  }

  //checks to see if the hostname is valid
  /*if( (hostServer = gethostbyaddr(argv[1], 4, AF_INET) ) == NULL)
  {
    cout<<"server "<<argv[1]<<" not found"<<endl;
    exit(1);
  }*/
  if( (hostServer = gethostbyname(argv[1]) ) == NULL)
  {
    cout<<"host "<<argv[1]<<" not found"<<endl;
    exit(1);
  }

  memcpy( hostServer->h_addr_list[0], (char*)&server_addr.sin_addr, hostServer->h_length );

  getTrackerFiles(tracker_files);
  //int useroutputid = pthread_create(&threads[threadCount], NULL, serverinput, NULL);//data from server
 // threadCount++;
  int serverinputid = pthread_create(&threads[threadCount], NULL, userinput, NULL);//commands from stdin
  threadCount++;
 // int peerinputid = pthread_create(&threads[threadCount], NULL, peerInput, NULL);//commands & data from peers
 // threadCount++;

  while(1)
  {
    //Allows the threads to do the real work
  }

  return 0;
}
