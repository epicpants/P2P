#include <iostream>
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
  cout<<"User Input Started"<<endl;

}

void *peerinput(void *threadid)
{
  cout<<"Peer Input Started"<<endl;
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

  return 0;
}