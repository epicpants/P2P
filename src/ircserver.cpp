/*
Authors:Chris Rawlings and Tyler Ryan  
Date:2013.11.14
Class:CS284
File:server.cpp
Purpose:server side of chat room
*/

#include <iostream>
#include <pthread.h> 
#include <signal.h>
#include <cstdlib>
#include <stdio.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>  /* define socket */
#include <netinet/in.h>  /* define internet socket */
#include <netdb.h>       /* define internet socket */
using namespace std;

#define SERVER_PORT 7777

const int MAX_CLIENT = 10;
int FD[MAX_CLIENT];
char usernames[MAX_CLIENT][500];
int counter = 0;
int numClients = 0;
int sd;
pthread_mutex_t m;

void* runClient(void* arg);

// Interrupt handler for Cntl-C
void signalHandler(int sig)
{
  cout << "  Ctrl-C detected. SERVER will shut down in 10 seconds..." << endl;
  char warning[] = "WARNING: SERVER will shut down in 10 seconds";
  
  // Warn the clients
  for(int i = 0; i < MAX_CLIENT; i++)
  {
    if(FD[i] > 0)
    {
      write(FD[i], warning, sizeof(warning));
    }
  }
  
  sleep(10);
  char code[]="bRZUkq3h173Uc31";
  
  // Kick the clients
  for(int i = 0; i < MAX_CLIENT; i++)
  {
    if(FD[i] > 0)
    {
      cout << usernames[i] << " has left" << endl;
      write(FD[i], code, sizeof(code));
    }
  }
  
  cout << "SERVER has exited!!!" << endl;
  close(sd);
  exit(1);
}

int main()
{
  
  struct sockaddr_in server_addr = { AF_INET, htons( SERVER_PORT ) };
  struct sockaddr_in client_addr = { AF_INET };
  unsigned int client_len = sizeof( client_addr );
  signal(SIGINT, signalHandler);
  
  for(int i = 0; i < MAX_CLIENT; i++)
  {
    FD[i] = 0;
  }
  /* create a stream socket */
  if( ( sd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
  {
    cerr << "SERVER: socket failed" << endl;
    exit( 1 );
  }
  
  /* bind the socket to an internet port */
  if( bind(sd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 )
  {
    cerr << "SERVER: bind failed" << endl;
    exit( 1 );
  }

  /* listen for clients */
  if( listen( sd, MAX_CLIENT-1 ) == -1 )
  {
    cerr << "SERVER: listen failed" << endl;
    exit( 1 );
  }

  cout << "SERVER is listening for clients to establish a connection\n";

  int temp;
  while((temp = accept(sd, (struct sockaddr*)&client_addr, &client_len )) > 0)
  {  
    // Check for max number of clients
    if(numClients < MAX_CLIENT && FD[counter] == 0)
    {
      pthread_mutex_lock(&m);
      FD[counter++] = temp;
      counter = counter % MAX_CLIENT;
      numClients++;
      if(numClients < MAX_CLIENT)
      {
        while(FD[counter] != 0)
        {
          counter = (counter + 1)%MAX_CLIENT;
        }
      }
      pthread_mutex_unlock(&m);
      pthread_t clientThread;
      pthread_create(&clientThread, NULL, runClient, &temp);
    }
    else if(numClients < MAX_CLIENT)
    {
      pthread_mutex_lock(&m);
      while(FD[counter] != 0)
      {
        counter = (counter + 1)%MAX_CLIENT;
      }
      FD[counter++] = temp;
      counter = counter % MAX_CLIENT;
      numClients++;
      if(numClients < MAX_CLIENT)
      {
        while(FD[counter] != 0)
        {
          counter = (counter + 1)%MAX_CLIENT;
        }
      }
      pthread_mutex_unlock(&m);
      pthread_t clientThread;
      pthread_create(&clientThread, NULL, runClient, &temp);
    }
    else
    {
      char error[] = "The max number of users are currently in the chatroom. Please try later\n";
      write(temp, error, sizeof(error));
    }
  }
  
 return 0;
}

// Thread for each client connection
void* runClient(void* arg) 
{
  if(arg == NULL)
  {
    cerr << "Thread received null argument" << endl;
  }
  
  char buffer[512];
  int k;
  int skt = *(int *)arg;
  int location;
  for(int i = 0; i < MAX_CLIENT; i++)
  {
    if(FD[i] == skt)
    {
      location = i;
    }
  }
  char username[500];
  char message[1024];
  
  //read in client name;
  if((k = read(skt, buffer, sizeof(buffer))) > 0)
  {   
    cout << buffer << " joined" << endl;
    strcpy(username, buffer);
    
    strcat(message, "Welcome ");
    strcat(message, username);
    //print a message about the new client;
    write(skt, message, sizeof(message));
  }
  
  //write message to each FD
  pthread_mutex_lock(&m);
  strcpy(message, "Client ");
  strcat(message, username);
  strcat(message, " has joined the chatroom");
  
  strcpy(usernames[location], username);
 
  for(int i = 0; i < MAX_CLIENT; i++)
  {
    if(FD[i] > 0 && FD[i] != skt)
    {
      write(FD[i], message, sizeof(message));
    }
  }
  pthread_mutex_unlock(&m);
    
  while ((k = read(skt, buffer, sizeof(buffer))) > 0)
  {
    pthread_mutex_lock(&m);
    strcpy(message, username);
    strcat(message, ": ");
    strcat(message, buffer);
    //write message to each FD
    for(int i = 0; i < MAX_CLIENT; i++)
    {
      if(FD[i] > 0 && FD[i] != skt)
      {
        write(FD[i], message, sizeof(message));
      }
    }
    pthread_mutex_unlock(&m);       
  }

  pthread_mutex_lock(&m);
  close(FD[location]);
  FD[location] = 0;
  cout << username << " has left" << endl;
  numClients--;
  pthread_mutex_unlock(&m);
  pthread_exit(arg);
  return NULL;
}
