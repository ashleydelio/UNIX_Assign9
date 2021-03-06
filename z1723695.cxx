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
 * 			file server. Can handle request
 * 			commands GET and INFO (Extra credit).
 * ********************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ctime>


using namespace std;

void processClientRequest(int connSock) {
	int received;
	char path[1024], buffer[1024];

	// read a message from the client
	if ((received = read(connSock, path, sizeof(path))) < 0) {
		perror("receive");
		exit(EXIT_FAILURE);
	}
	cout << "Client request: " << path << endl;
	
	//cout << "Pathname: " << pathname << endl;

//breakup path into "GET" command request and actual pathname of directory or file
	const char delim[2] = " ";
	char * section[3];
	int j = 0;
	
	char *token;
	string reqCommand, pathname;
	
	token = strtok(path, delim);

//Place	pieces of path (separated by whitespace) into section array and corresponding reqCommand and pathname strings
	while (token != NULL){
		section[j] = (char *) malloc  (strlen(token) + 1);
		strcpy(section[j], token);
		
		
		if (j == 0){
			reqCommand = section[j];
		}
		
		else if (j == 1){
			pathname = section[j];
		}
		
		j++;
		
		token = strtok(NULL, delim);
	}

//If requested command is GET	
	if(reqCommand == "GET"){
	struct stat s;
	if( stat(pathname.c_str(),&s) == 0 ){
		if( s.st_mode & S_IFDIR ){
        //it's a directory
        
        cout << "It's a directory" << endl;
        
        // open directory
		DIR *dirp = opendir(pathname.c_str());
		if (dirp == 0) {
		// tell client that an error occurred
		// duplicate socket descriptor into error output
			close(2);
			dup(connSock);
			perror(pathname.c_str());
			exit(EXIT_SUCCESS);
		}
	
	// read directory entries
		
		struct dirent *dirEntry;
		while ((dirEntry = readdir(dirp)) != NULL) {
			
			if (dirEntry->d_name[0] != '.') {
        	strcpy(buffer, dirEntry->d_name);
			strcat(buffer, "\n");
			}
		
			else{
				continue;
			}
			
			//write back to client
			if (write(connSock, buffer, strlen(buffer)) < 0) {
				perror("write");
				exit(EXIT_FAILURE);
			}
			cout << "sent: " << buffer;		
			}	
		
		closedir(dirp);
		cout << "done with client request\n";
		close(connSock);
		exit(EXIT_SUCCESS);
        
		}
		else if( s.st_mode & S_IFREG ){
        //it's a file
        
        cout << "It's a file" << endl;
      
		FILE * pFile;
		size_t lSize;
		char * source;
		size_t result;

		pFile = fopen ( pathname.c_str() , "rb" );
		if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

		// obtain file size:
		fseek (pFile , 0 , SEEK_END);
		lSize = ftell (pFile);
		rewind (pFile);

		// allocate memory to contain the whole file:
		source = (char*) malloc (sizeof(char)*lSize);
		if (source == NULL) {fputs ("Memory error",stderr); exit (2);}

		// copy the file into the buffer:
		result = fread (source,1,lSize,pFile);
		if (result != lSize) {fputs ("Reading error",stderr); exit (3);}

  /* the whole file is now loaded in the memory buffer. */

		// terminate
		fclose (pFile);
		        
        if (write(connSock, source, strlen(source)) < 0) {
			perror("write");
			exit(EXIT_FAILURE);
		}
        
        free(source);
        close(connSock);
		exit(EXIT_SUCCESS);
        
	}
}
	else{
		char error[1024];
		strcpy(error, "Error: ");
		strcat(error, pathname.c_str());
		strcat(error, " not found\n");
		
		if (write(connSock, error, strlen(error)) < 0) {
			perror("write");
			exit(EXIT_FAILURE);
		}
		
		close(connSock);
		exit(EXIT_SUCCESS);
	}
}

//Else if requested command is "INFO"

	else if(reqCommand == "INFO"){
	
	//current date/time
	time_t now = time(0);
	
	//convert now to string
	char* dt = ctime(&now);
	if (write(connSock, dt, strlen(dt)) < 0) {
			perror("write");
			exit(EXIT_FAILURE);
		}
        
        close(connSock);
		exit(EXIT_SUCCESS);
	
	
	}
	
}	

        
int main(int argc, char *argv[]) {

	if (argc != 2) {
		cerr << "USAGE: Z1723695_Server port\n";
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
	
	cout << "Z1723695_Server listening on port: " << argv[1] << endl;

	// Run until cancelled 
	while (true) {
		int connSock=accept(sock, (struct sockaddr *) &client_address, &addrlen);
		if (connSock < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
		// fork
		if (fork()) { 	    // parent process
			close(connSock);
		} else { 			// child process
			processClientRequest(connSock);
		}
	}	
	close(sock);
	return 0;
}
