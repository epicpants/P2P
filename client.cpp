#include <iostream>
#include <sstream>
#include <pthread.h> 
#include <signal.h>
#include <cstdlib>
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>  /* define socket */
#include <netinet/in.h>  /* define internet socket */
#include <netdb.h>       /* define internet socket */
#include "tracker_parser.h"
using namespace std;

#define SERVER_PORT 7777

bool createTracker(string fileName)
{
  //check if tracker already exists
  //check if file exists
  //make tracker
  //update tracker server
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

void *userinput(void *threadid)
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

void *peerinput(void *threadid)
{
  
}

void *getfrompeer(void *threadid)
{

}


int main(int argc, char* argv[])
{
  pthread_t threads[65535];
  unsigned int threadCount = 0;
  int userinputid = pthread_create(&threads[threadCount], NULL, userinput, NULL);
  threadCount++;
  int peerinputid = pthread_create(&threads[threadCount], NULL, peerinput, NULL);
  threadCount++;

  while(1)
  {

  }

  return 0;
}