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
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>  /* define socket */
#include <netinet/in.h>  /* define internet socket */
#include <netdb.h>       /* define internet socket */
using namespace std;

#define SERVER_PORT 7777

int sd;
void* runPeer(void* arg);


int main()
{
  
  struct sockaddr_in server_addr = { AF_INET, htons( SERVER_PORT ) };
  struct sockaddr_in client_addr = { AF_INET };
  unsigned int client_len = sizeof( client_addr );
  
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
  if( listen( sd, 1 ) == -1 )
  {
    cerr << "SERVER: listen failed" << endl;
    exit( 1 );
  }

  cout << "SERVER is listening for clients to establish a connection\n";

  int temp;
  while((temp = accept(sd, (struct sockaddr*)&client_addr, &client_len )) > 0)
  {  
	pthread_t peerThread;
	pthread_create(&peerThread, NULL, runPeer, &temp);
  }
  
 return 0;
}

// Thread for each client connection
void* runPeer(void* arg) 
{
  if(arg == NULL)
  {
    cerr << "Thread received null argument" << endl;
  }
  
  int skt = *(int *)arg;
  char buffer[512];
  int length;
  
  while ((length = read(skt, buffer, sizeof(buffer))) > 0)
  {
	buffer[length] = '\0';
	if((!strcmp(buffer, "REQ LIST"))||(!strcmp(buffer, "req list"))||(!strcmp(buffer, "<REQ LIST>"))||(!strcmp(buffer, "<REQ LIST>\n"))){//list command received
		//handle_list_req(skt);// handle list request
		//printf("list request handled.\n");
	}
	else if((strstr(buffer,"get")!=NULL)||(strstr(buffer,"GET")!=NULL)){// get command received
		//xtrct_fname(buffer, " ");// extract filename from the command		
		//handle_get_req(skt, fname);		
	}
	else if((strstr(buffer,"createtracker")!=NULL)||(strstr(buffer,"Createtracker")!=NULL)||(strstr(buffer,"CREATETRACKER")!=NULL)){// get command received
		//tokenize_createmsg(buffer);
		//handle_createtracker_req(skt);
		
	}
	else if((strstr(buffer,"updatetracker")!=NULL)||(strstr(buffer,"Updatetracker")!=NULL)||(strstr(buffer,"UPDATETRACKER")!=NULL)){// get command received
		//tokenize_updatemsg(buffer);
		//handle_updatetracker_req(skt);		
	}
  }
  
  pthread_exit(arg);
  return NULL;
}
