/* *********************************************
 * Programmer Name: Ashley De Lio
 * 
 * Z-ID: Z1723695
 * 
 * Class: CSCI 330 - 2
 * 
 * Program:  Assignment 9
 * 
 * Purpose: Exercise TCP server socket system 
 * 			calls. Program implements a simple 
 * 			file server.
 * ********************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
using namespace std;
        
int main(int argc, char *argv[]) {
	// check arguments
	if (argc != 2) {
		cerr << "USAGE: echoTCPServer port\n";
		exit(EXIT_FAILURE);
	}
		
	// Create the TCP socket 
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}	
	// create address structures
	struct sockaddr_in server_address;  // structure for address of server
	struct sockaddr_in client_address;  // structure for address of client
	unsigned int addrlen = sizeof(client_address);	

	// Construct the server sockaddr_in structure 
	memset(&server_address, 0, sizeof(server_address));   /* Clear struct */
	server_address.sin_family = AF_INET;                  /* Internet/IP */
	server_address.sin_addr.s_addr = INADDR_ANY;          /* Any IP address */
	server_address.sin_port = htons(atoi(argv[1]));       /* server port */

	// Bind the socket
	if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}	

	// listen: make socket passive and set length of queue
	if (listen(sock, 64) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}	
	
	cout << "echoServer listening on port: " << argv[1] << endl;

	// Run until cancelled 
	
	int connSock[11];
	int i = 0;
	int rs;
	while (true) {
		connSock[i]=accept(sock, (struct sockaddr *) &client_address, &addrlen);
		if (connSock[i] < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
		
//Fork

		int pid;
		// fork will make 2 processes
		pid = fork();
		if (pid == -1) { perror("fork"); exit(EXIT_FAILURE); }
 
		if (pid == 0) {
		// Child process: 		
			
			// read a message from the client
			char buffer[1024];
			int received = read(connSock[i], buffer, sizeof(buffer));
			if (received < 0) {
			perror("read");
			exit(EXIT_FAILURE);
			}
			//GET
		
		
		cout << "Buffer: " << buffer << endl;
//Begin process to divide request command and pathname string into reqCommand and path strings

//Define deliminator and new array for reqCommand and path to be placed into	
	const char delim[2] = " ";
	char * section[3];
	int j = 0;
	
	char *token;
	string reqCommand, path;
	
	token = strtok(buffer, delim);

//Place	pieces of argv[2] (separated by whitespace) into section array and corresponding reqCommand and path strings
	while (token != NULL){
		section[j] = (char *) malloc  (strlen(token) + 1);
		strcpy(section[j], token);
		
		
		if (j == 0){
			reqCommand = section[j];
		}
		
		else if (j == 1){
			path = section[j];
		}
		
		j++;
		
		token = strtok(NULL, delim);
	}
cout << "reqCommand is: " << reqCommand << endl;
cout << "path is: " << path << endl;
//if request command is GET, get and write back appropriate file or directory		
		if (reqCommand == "GET"){
			rs = execlp("/bin/ls", "/bin/ls", path.c_str(), (char*) NULL);
			if (rs == -1) { 
				perror(reqCommand.c_str()); exit(EXIT_FAILURE); 
			}
			
			
			
			// write the message back to client 
			if (write(connSock[i], buffer, received) < 0) {
				perror("write");
				exit(EXIT_FAILURE);
			}
			close(connSock[i]);
		
		}
	
//if request command is INFO, get and write back the current date and time in text format	
		else if (reqCommand == "INFO"){
		
	
				
		
		
		
			// write the message back to client 
			if (write(connSock[i], buffer, received) < 0) {
				perror("write");
				exit(EXIT_FAILURE);
			}
			
			
			
			close(connSock[i]);
	}
	
	}
	else { 
      // Parent process: continue to top of loop
      i++;
      if(i > 10){
		i = 0;
	  }
      continue;
		
	}
	
}	
	close(sock);
	return 0;
}
