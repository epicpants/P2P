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

int clientSocket;
int k;
char buff[1024];
char buffer[1024];
char code[]="bRZUkq3h173Uc31";
bool exitCondition=false;

void * readingOut(void* arg);

int main(int argc, char* argv[])
{
  pthread_mutex_t m;
  char quit[]="/quit";
  char exitb[]="/exit";
  char part[]="/part";
  struct hostent *hostServer;
  struct sockaddr_in server_addr = { AF_INET, htons( SERVER_PORT ) };

  //checks to see if the correct number of arguments were passed in
  if(argc != 2)
  {
    cout<<"Usage: "<<argv[0]<<" hostname"<<endl;
    exit(1);
  }

  //checks to see if the hostname is valid
  if( (hostServer = gethostbyname(argv[1]) ) == NULL)
  {
    cout<<"host "<<argv[1]<<" not found"<<endl;
    exit(1);
  }
  
  memcpy( hostServer->h_addr_list[0], (char*)&server_addr.sin_addr, hostServer->h_length );
    
  //creating a socket for the client
  if( (clientSocket=socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    cout<<"Socket Creation Failure"<<endl;
    exit(1);
  }
  
  //client connecting to a socket
  if( (connect( clientSocket,(struct sockaddr*)&server_addr, sizeof(server_addr))) == -1)
  {
    cout<<"Connection Failed"<<endl;
    exit(1);
  }
  
  //get username from user
  cout<<"Connection Successful"<<endl;
  cout<<"Enter Username: ";
  cin>>buff;
  write(clientSocket, buff, sizeof(buff));
  read(clientSocket, buff, sizeof(buff));
  cout << buff << endl;

  //create thread for handling simultaneous input & output
  pthread_t readThread;
  pthread_create(&readThread, NULL, readingOut, &clientSocket);
  
   cin.ignore(1000, '\n');
   
   //output to server
  while(exitCondition==false)
  {
    string stupidStringVar="";
    getline(cin, stupidStringVar);
    strcpy(buff, stupidStringVar.c_str());
    
    if(strcmp(buff, quit)==0 || strcmp(buff, exitb)==0 || strcmp(buff, part)==0)
    {
      exitCondition=true;
      cout << "Exited!!!" << endl;
      exit(1);
    }
    else
    {
      write(clientSocket, buff, stupidStringVar.size()*8);
    }
  }
  
  close(clientSocket);
  return 0;
}

//input from server
void* readingOut(void* arg)
{
  int clientSocket = *(int *)arg;
  
  while ((k = read(clientSocket, buffer, sizeof(buffer))) > 0)
  {
    if(strcmp(buffer, code) == 0)
    {
      exitCondition=true;
      cout<<"This program will now terminate"<<endl;
      sleep(1);
      cout<<"Exited!!!"<<endl;
      exit(1);
    }
    cout<<buffer<<endl;
  }
  
  return NULL;
}