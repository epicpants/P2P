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
#include <dirent.h>
#include <vector>
#include "tracker_parser.h"
#include <sys/types.h>
#include <sys/socket.h>  /* define socket */
#include <netinet/in.h>  /* define internet socket */
#include <netdb.h>       /* define internet socket */
using namespace std;

#define SERVER_PORT 7777

int sd;
void* runPeer(void* arg);
void getTrackerFiles(vector<string> & tracker_list_out);

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
  char buffer[1024];
  char response[1024];
  int length;
  
  length = read(skt, buffer, sizeof(buffer));
  buffer[length] = '\0';
  if((!strcmp(buffer, "REQ LIST"))||(!strcmp(buffer, "req list"))||(!strcmp(buffer, "<REQ LIST>"))||(!strcmp(buffer, "<REQ LIST>\n"))){//list command received
    cout<<"Client requested tracker file list"<<endl;
    vector<string> tracker_files;
    getTrackerFiles(tracker_files);
    unsigned int num_tracker_files = tracker_files.size();
    strcpy(response, "REP LIST ");
    ostringstream numToString;
    numToString<<num_tracker_files;
    const char* num_files = numToString.str().c_str();
    strcat(response, num_files);
    write(skt, response, sizeof(response));
    
    for(unsigned int i = 0; i < num_tracker_files; i++)
    {
      TrackerFile tracker_file;
      const char* name = tracker_files[i].c_str();
      if(!tracker_file.parseTrackerFile(name))
      {
        long filesize = tracker_file.getFilesize();
        string md5 = tracker_file.getMD5();
        const char* file_md5 = md5.c_str();
        
        ostringstream file_num_ss;
        file_num_ss.str("");
        file_num_ss<<(i+1);
        const char* file_num = file_num_ss.str().c_str();
        strcpy(response, file_num);
        strcat(response, " ");
        strcat(response, name);
        strcat(response, " ");
        
        ostringstream file_size_ss;
        file_size_ss.str("");
        file_size_ss<<filesize;
        const char* file_size = file_size_ss.str().c_str();
        strcat(response, file_size);
        strcat(response, " ");
        strcat(response, file_md5);
        write(skt, response, sizeof(response));
      }
      else // 
      {
        cerr<<"Error parsing tracker file"<<endl;
        strcpy(response, "Error");
        write(skt, response, sizeof(response));
      }
    }
    
    strcpy(response, "REP LIST END");
    write(skt, response, sizeof(response));
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
  
  pthread_exit(arg);
  return NULL;
}
