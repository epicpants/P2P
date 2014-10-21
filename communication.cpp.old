#include "communication.h"


DataSender::DataSender(string serverIP = "", unsigned short serverPort = 0){
	
	// Initialize server info
	this->mySocket = 0;
	this->serverPort = serverPort;
	this->serverIP = serverIP;
}


// Getters & Setters for IP and port
string DataSender::getServer() {
	return serverIP;
}
	
unsigned short DataSender::getPort() {
	return serverPort;
}
	

void DataSender::setServer(string serverIP) {
	this->serverIP = serverIP;
	return;
}
		
void DataSender::setPort(unsigned int serverPort) {
	this->serverPort = serverPort;
	return;
}

/* Desc: Used to establish remote TCP connection
 * Input: serverIP expects IPv4 address as string, ex: "192.168.1.1"
*/
short DataSender::createConn() {
	
	short result = -1;
	
	if ( ( serverIP.length() > 0 ) && ( serverPort != 0 ) ) {
	
		// Create struct for holding remote host's address info
		struct sockaddr_in remoteHost;
	
		// Zero out address struct
		memset(&remoteHost, 0, sizeof(remoteHost));
	
		// Populate address struct with parameters
		remoteHost.sin_family = AF_INET; //IPv4
		remoteHost.sin_addr.s_addr = inet_addr(serverIP.c_str());
		remoteHost.sin_port = htons(serverPort);
	
		// create new IPv4 TCP stream socket
		mySocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	
		// connect to remote host via socket
		result = connect(mySocket, (struct sockaddr*) &remoteHost,
			sizeof(remoteHost));

		//create thread for handling simultaneous input & output
		pthread_t readThread;
		pthread_create(&readThread, NULL, DataSender::ReplyListener, &mySocket);
	}
	
	return result;
}

//input from server
void* DataSender::ReplyListener(void* arg) {
	
	int clientSocket = *(int *)arg;
	
	int recvMsgLength = read(clientSocket, replyBuffer, sizeof(replyBuffer));
	
	while ( recvMsgLength > 0 ) {

		recvMsgLength = read(clientSocket, replyBuffer, sizeof(replyBuffer)
		
	}
	
	cout<<replyBuffer<<endl;
	disconnect();
	pthread_exit(arg);
	return NULL;
}

// Desc: Disconnect TCP connection and reset mySocket
short DataSender::disconnect() {
	short result = 0;
	
	if( mySocket ) {
		result = close( mySocket );
		mySocket = 0;
	}
	
	return result;
}


/* Desc: Transmits string message less than 1024 bytes to
 * 		current socket connection
 * Input: Expects string of less than 1024 bytess
*/
short DataSender::transmit(string message = "") {
	
	short result = -1;
	
	if( ( message.length() <= MAXMESGLEN ) && ( mySocket > 0 ) ) {
		
		// sends message, converting to C style string
		result = send(mySocket, message.c_str(), message.length(), 0);
	}
	
	return result;
}

DataReceiver::DataReceiver() {
	// Initialize server info
	listenSocket = 0;
	clientSocket = 0;
	myIP = INADDR_ANY;
}

// For listening and receiving data
void DataReceiver::listener(int portNum = CLIENTPORT) {
	
	// Create struct for holding local host's address info
	struct sockaddr_in localHost, clientAddr;

	// Zero out address struct
	memset(&localHost, 0, sizeof(localHost));

	// Populate address struct with parameters
	localHost.sin_family = AF_INET; //IPv4
	localHost.sin_addr.s_addr = htonl(myIP);
	localHost.sin_port = htons(portNum);
	
	// create new IPv4 TCP stream socket
	listenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	// bind to socket
	bind(listenSocket, (struct sockaddr*) &localHost, sizeof(localHost));
	
	// listen for incoming connections
	listen(listenSocket, LISTENQUEUE);
	
	// accept
	clientSocket = accept(listenSocket, (struct sockaddr*) &clientAddr, sizeof(clientAddr) );
	
	HandleTCPClient();
	
	return;
}

void DataReceiver::HandleTCPClient() {
	
	int recvMsgSize = 0;
	
	// Receive message from client
	recvMsgSize = recv( clientSocket, msgBuffer, RCVBUFSIZE, 0 );
	
	while( recvMsgSize > 0 ) {
		
		// send comm flag?
		
		recvMsgSize = recv(clientSocket, msgBuffer, RCVBUFSIZE, 0 );
		
	}
	
	return;
}


short DataReceiver::transmit(string message = "") {
	
	short result = -1;
	
	if( ( message.length() <= MAXMESGLEN ) && ( clientSocket > 0 ) ) {
		
		// sends message, converting to C style string
		result = send(clientSocket, message.c_str(), message.length(), 0);
	}
	
	return result;
}
