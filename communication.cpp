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
	}
	
	return result;
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
