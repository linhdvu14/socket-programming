/*********************************************************************
** ftserver-utils.c
** Description: Server utility functions
*********************************************************************/

#include "ftserver-utils.h"


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
* readArgs()
* Description: Validates and reads command line args
* Params:
*	argc: number of command line args
*	argv[]: array of command line args
* Returns: Port number passed
*********************************************************************/
int readArgs(int argc, char* argv[]) {
	int portno;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s [portno]\n", argv[0]);
		exit(1);
	}
	
	portno = atoi(argv[1]);
	return portno;
}


/*********************************************************************
* readRequest()
* Description: Processes and reads client's request into args
* Params:
*	request: string of client's request, fields separated by space
*	cmd: where to store client's command [-g or -l]
*	fn: where to store requested file name
*	dataPort: where to store data port number
* Returns: 0 if command valid, -1 if invalid.
*********************************************************************/
int readRequest(char* request, char* cmd, char* fn, int* dataPort) {
	char* tok;

	/* Read first string into cmd */
	tok = strtok(request, " ");
	strcpy(cmd, tok);
	
	/* Read remaining strings into fn and dataPort */
	tok = strtok(NULL, " ");		
	if (strcmp(cmd, "-g") == 0) {
		strcpy(fn, tok);
		tok = strtok(NULL, " ");
		*dataPort = atoi(tok);
	} else if (strcmp(cmd, "-l") == 0) {
		*dataPort = atoi(tok);
	
	/* Validate request */
	} else {
		return -1;
	}

	tok = strtok(NULL, " ");
	if (tok != NULL) {
		return -1;
	}

	/* Return 0 if valid */
	return 0;
}


/*********************************************************************
* startUpSocket()
* Description: Starts up socket for accepting connections
# Params:
#	portno: port number
# Returns: Socket file descriptor
*********************************************************************/
int startUpSocket(int portno) {
	int serverSocket;
	struct sockaddr_in server;

	/* Create socket */
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0) {
		error("Creating socket");
	}

	/* Configure server */
	memset(&server, '\0', sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(portno);
	server.sin_addr.s_addr = INADDR_ANY;

	/* Bind port to socket */
	if (bind(serverSocket, (struct sockaddr *) &server, sizeof(server)) < 0) {
		error("Binding port to socket");
	}

	return serverSocket;
}


/*********************************************************************
* acceptConnection()
* Description: Accepts new conection. Exchanges identities.
* Params:
*	serverSocket: server socket file descriptor
*	clientName: where to store client name
* Returns: Socket file descriptor.
*********************************************************************/
int acceptConnection(int serverSocket, char* clientName) {
	int clientSocket;
	struct hostent* h;
	char serverName[NAME_LENGTH];

	/* Accept new connection */
	clientSocket = accept(serverSocket, NULL, NULL);
	if (clientSocket < 0) {
		error("Accepting connection");
	}

	/* Receive client name */
	safeRead(clientSocket, clientName, NAME_LENGTH);
	printf("\nNew connection established with %s.\n", clientName);

	/* Send server name */
	memset(serverName, '\0', NAME_LENGTH);
	gethostname(serverName, NAME_LENGTH);
	h = gethostbyname(serverName);
	safeWrite(clientSocket, h->h_name);

	return clientSocket;
}


/*********************************************************************
* initConnection()
* Description: Initiates connection with already running host
* Params:
*	hostname: host to be contacted
*	portno: port number
* Returns: Socket file descriptor
*********************************************************************/
int initConnection(char* hostname, int portno) {
	int sockfd;
	struct sockaddr_in host;
	struct hostent* host_ip_addr;

	/* Create socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("Creating socket");
	}

	/* Configure */
	host_ip_addr = gethostbyname(hostname);
	memset(&host, '\0', sizeof(host));
	host.sin_family = AF_INET;
	host.sin_port = htons(portno);
	memcpy(&host.sin_addr, host_ip_addr->h_addr, host_ip_addr->h_length);

	/* Initiate connection */
	if (connect(sockfd, (struct sockaddr *) &host, sizeof(host)) < 0) {
		error("Contacting host");
	}

	return sockfd;
}


/*********************************************************************
* safeRead()
* Description: Reads deliminated message from socket
* Params:
* 	sockfd: socket file descriptor
* 	msg: where to write msg to
*	msglen: buffer length
* Returns: 0 if success, -1 if failure.
*********************************************************************/
int safeRead(int sockfd, char* msg, int msglen) {
	int n, idx = 0;
	char c;

	/* Look for start delim char */
	do {
		n = read(sockfd, &c, 1);
		if (n < 0) {
			return -1;
		}
	} while (c != DELIM_S);

	/* Start delim found, reset buffer and read msg */
	memset(msg, '\0', msglen);
	do {
		/* Read each char into c */
		n = read(sockfd, &c, 1);
		if (n < 0) {
			return -1;
		}

		/* Stop when reach end delim char */
		if (c == DELIM_E) {
			break;
		}

		/* Update buffer location */
		msg[idx] = c;
		idx++;
	} while (1);

	return 0;
}


/*********************************************************************
* safeWrite()
* Description: Writes message to socket, prefixed with start delim char
*	and suffixed with end delim char. Ensures full message gets written
*	by looping until message size reached.
* Params:
* 	sockfd: socket file descriptor
* 	msg: msg to be sent
* Returns: 0 if success, -1 if failure.
*********************************************************************/
int safeWrite(int sockfd, char* msg) {
	char delim_s_str[2] = "\0", delim_e_str[2] = "\0";
	int bytes_written = 0, n;

	/* Build strings of DELIM_S and DELIM_E */
	delim_s_str[0] = DELIM_S;
	delim_e_str[0] = DELIM_E;

	/* Send start delim char */
	n = write(sockfd, delim_s_str, 1);
 	if (n < 0) {
 		error("Writing to socket");
 	}

	/* Send message */
	while (bytes_written != strlen(msg)) {
		n = write(sockfd, &msg[bytes_written], strlen(msg) - bytes_written);
		if (n < 0) {
			return -1;
		}
		bytes_written += n;
	}

	/* Send end delim char */
	n = write(sockfd, delim_e_str, 1);
 	if (n < 0) {
 		error("Writing to socket");
 	}

	return 0;
}


/*********************************************************************
* safeWriteFile()
* Description: Writes file content to socket, deliminated with delim
*	chars. Loops until full message written.
* Params:
* 	sockfd: socket file descriptor
* 	file: file to be sent
* Returns: None.
*********************************************************************/
void safeWriteFile(int sockfd, FILE* file) {
	long file_size;
	char* buffer;
	int n;

	/* Get file size */
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	/* Read file content to buffer */
	buffer = malloc(file_size + 1);
	fread(buffer, file_size, 1, file);

	/* Send buffered message */ 
	do {
		n = safeWrite(sockfd, buffer);
	} while (n < 0);

	free(buffer);
}
