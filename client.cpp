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

}

void *peerinput(void *threadid)
{

}

void *getfrompeer(void *threadid)
{

}


int main(int argc, char* argv[])
{
  pthread_t threads[4294967295];
  unsigned int threadcount = 0;
  int userinputid = pthread_create(&threads[threadcount], NULL, userinput, NULL);
  threadcount++;

}