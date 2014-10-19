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

#define SERVER_PORT 7777

using namespace std;

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