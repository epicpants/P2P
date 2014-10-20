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
#include "md5.h"
#include <sys/types.h>
#include <sys/socket.h>  /* define socket */
#include <netinet/in.h>  /* define internet socket */
#include <netdb.h>       /* define internet socket */
using namespace std;

#define SERVER_PORT 7777
#define CHUNK_SIZE 1024

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
  if( listen( sd, 10 ) == -1 )
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
        const char* file_name = tracker_file.getFilename().c_str();
        
        ostringstream file_num_ss;
        file_num_ss.str("");
        file_num_ss<<(i+1);
        const char* file_num = file_num_ss.str().c_str();
        strcpy(response, file_num);
        strcat(response, " ");
        strcat(response, file_name);
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
    
    stringstream ss(buffer);
    string file_name;
    FILE* file;
    long tracker_filesize;
    char* file_contents;
    ss >> file_name >> file_name;
    cout << "Client requested GET for " << file_name << endl;
    strcpy(response, "REP GET BEGIN");
    write(skt, response, sizeof(response));
    
    file = fopen(file_name.c_str(), "r");
    if(file != NULL)
    {
      fseek(file, 0L, SEEK_END);
      tracker_filesize = ftell(file);
      rewind(file);
      long bytes_read = 0;
      file_contents = (char*) malloc(sizeof(char) * tracker_filesize);
      while(bytes_read < tracker_filesize)
      {
        if(bytes_read + CHUNK_SIZE > tracker_filesize)
        {
          bytes_read += fread(response, sizeof(char), tracker_filesize % CHUNK_SIZE, file);
          response[tracker_filesize % CHUNK_SIZE] = '\0';
        }
        else
        {
          bytes_read += fread(response, sizeof(char), CHUNK_SIZE, file);
        } 
        write(skt, response, sizeof(response));
        strcat(file_contents, response);
      }
    }
    fclose(file);
    string contents = file_contents;
    MD5 md5(contents);
    const char* file_md5 = md5.hexdigest().c_str();
    strcpy(response, "REP GET END ");
    strcat(response, file_md5);
    write(skt, response, sizeof(response));
  }
  else if((strstr(buffer,"createtracker")!=NULL)||(strstr(buffer,"Createtracker")!=NULL)||(strstr(buffer,"CREATETRACKER")!=NULL)){// get command received
    TrackerFile tracker_file;
    stringstream ss(buffer);
    string temp;
    vector<string> tokens;
    while(ss >> temp)
    {
      tokens.push_back(temp);
    }
    unsigned int num_tokens = tokens.size();
    string ipaddr = tokens[num_tokens-2];
    string port = tokens[num_tokens-1];
    cout << "Client " << ipaddr << ":" << port << " requested createtracker for " << tokens[1] << endl;
    int create_status = FILE_FAIL;
    try {
      create_status = tracker_file.create(buffer);
    }
    catch(exception& ex)
    {
      cerr<<ex.what();
    }
    strcpy(response, "createtracker ");
    if(create_status == FILE_SUCC)
    {
      strcat(response, " succ");
    }
    else if(create_status == FILE_ERR)
    {
      strcat(response, " ferr");
    }
    else
    {
      strcat(response, " fail");
    }
    write(skt, response, sizeof(response));
  }
  else if((strstr(buffer,"updatetracker")!=NULL)||(strstr(buffer,"Updatetracker")!=NULL)||(strstr(buffer,"UPDATETRACKER")!=NULL)){// get command received
    TrackerFile tracker_file;
    string file_name;
    string ipaddr;
    string port;
    stringstream ss(buffer);
    // Don't care about "updatetracker", "start_byte", or "end_byte" tokens for console output
    ss >> file_name >> file_name >> ipaddr >> ipaddr >> ipaddr >> port;
    cout << "Client " << ipaddr << ":" << port << " requested updatetracker for " << file_name << endl;
    const char* fname = file_name.c_str();
    int update_status = FILE_FAIL;
    try {
      update_status = tracker_file.update(buffer);
    }
    catch(exception& ex)
    {
      cerr<<ex.what();
    }
    strcpy(response, "updatetracker ");
    strcat(response, fname);
    if(update_status == FILE_SUCC)
    {
      strcat(response, " succ");
    }
    else if(update_status == FILE_ERR)
    {
      strcat(response, " ferr");
    }
    else
    {
      strcat(response, " fail");
    }
    write(skt, response, sizeof(response));
  }
  
  pthread_exit(arg);
  return NULL;
}
