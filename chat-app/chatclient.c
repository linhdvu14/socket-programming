/*********************************************************************
** chatclient.c
** Description: Client side of client-server network chat application 
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>

#define BUFFER_SIZE 501
#define HANDLE_SIZE 11


/*********************************************************************
* error()
* Description: Prints error message and exits with code 1
* Params:
*	msg: error message
* Returns: None.
*********************************************************************/
void error(const char* msg) {
    perror(msg);
    exit(1);
}


/*********************************************************************
* startUp()
* Description: Gets command line args, starts up socket, initiates
*	contact with server
* Params:
*	argc: number of command line args
*	argv[]: array of command line args
* Returns: Connection socket file descriptor
*********************************************************************/
int startUp(int argc, char* argv[]) {
	int sock, portno, n;
	struct sockaddr_in server;
	struct hostent* server_ip_addr;
	char hostname[50];

	/* Check number of arguments */
	if (argc != 3) {
		fprintf(stderr, "Usage: %s [hostname] [port number]\n", argv[0]);
		exit(1);
	}

	/* Set hostname and port number */
	strcpy(hostname, argv[1]);
	portno = atoi(argv[2]);

	/* Create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		error("Creating socket");

	/* Configure server */
	server_ip_addr = gethostbyname(hostname);
	memset(&server, '\0', sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(portno);
	memcpy(&server.sin_addr, server_ip_addr->h_addr, server_ip_addr->h_length);

	/* Connect to server */
	if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
		error("Contacting server");
	printf("Connection established.\n");

	return sock;
}


/*********************************************************************
* sendMessage()
* Description: Reads user input, send over connection
* Params:
*	sock: socket over which to exchange message
*	buffer: message buffer
*	c_handle: client's handle
* Returns: 1 if client enters "/quit", 0 otherwise
*********************************************************************/
int sendMessage(int sock, char buffer[BUFFER_SIZE], char c_handle[HANDLE_SIZE]) {
	int n;

	/* Get client's message */
	printf("%s> ", c_handle);
	memset(buffer, '\0', BUFFER_SIZE);
	fgets(buffer, BUFFER_SIZE, stdin);
	strtok(buffer, "\n");
	
	/* If client quits, notify server */
	if (strcmp(buffer, "/quit") == 0) {
		n = write(sock, buffer, strlen(buffer));
 		if (n < 0) 
 			error("Writing to socket");
		return 1;
	}
	
	/* Write client's message to server */
	n = write(sock, buffer, strlen(buffer));
 	if (n < 0) 
 		error("Writing to socket");	

 	return 0;
}


/*********************************************************************
* receiveMessage()
* Description: Receives and displays server's message
* Params:
*	sock: socket over which to exchange message
*	buffer: message buffer
*	s_handle: server's handle
* Returns: 1 if server enters "/quit", 0 otherwise
*********************************************************************/
int receiveMessage(int sock, char buffer[BUFFER_SIZE], char s_handle[HANDLE_SIZE]) {
	int n;
 	
 	/* Read message from server */
 	memset(buffer, '\0', BUFFER_SIZE);
 	n = read(sock, buffer, BUFFER_SIZE);
 	if (n < 0)
 		error("Reading from socket");
 	strtok(buffer, "\n"); 

 	/* If server quits, notify client */    
	if (strcmp(buffer, "/quit") == 0) {
		return 1;
	}

	/* Display server's message */
 	printf("%s> %s\n", s_handle, buffer);
 	return 0;		
}




/*********************************************************************
** MAIN **/

int main(int argc, char *argv[]) {
	char s_handle[HANDLE_SIZE], c_handle[HANDLE_SIZE], buffer[BUFFER_SIZE];
	int n;
	int sock = startUp(argc, argv);

	/* Get client's handle from user */
	printf("What's your name?> ");
	memset(c_handle, '\0', HANDLE_SIZE);
	fgets(c_handle, HANDLE_SIZE, stdin);
	strtok(c_handle, "\n");
	
	/* Exchange handle info */
	memset(s_handle, '\0', HANDLE_SIZE);
 	n = read(sock, s_handle, HANDLE_SIZE);  // read server's handle
 	if (n < 0)
 		error("Reading from socket");
	n = write(sock, c_handle, strlen(c_handle));  // write client's handle
 	if (n < 0) 
 		error("Writing to socket");
       
	/* Exchange messages over connection until either host quits */
	while (1) {
		/* Send client's message */
		if (sendMessage(sock, buffer, c_handle) == 1) {
			break;
		}
     		
     	/* Receive server's message */
     	if (receiveMessage(sock, buffer, s_handle) == 1) {
     		break;
     	}
	}
	
	/* Either host has quit, close connection and exit */
	close(sock);
	printf("Connection closed.\n");
	exit(0);
}
