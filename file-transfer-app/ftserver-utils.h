/*********************************************************************
** ftserver-utils.h
** Description: Server utility functions
*********************************************************************/

#ifndef FTSEVER_UTILS_H_
#define FTSEVER_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>

#define BUFFER_LENGTH 4096
#define QUEUE_SIZE 5
#define NAME_LENGTH 500
#define MSG_INVALID_CMD "INVALID COMMAND"
#define MSG_FILE_NOT_FOUND "FILE NOT FOUND"
#define MSG_OK "OK"

#define DELIM_S '\x2'
#define DELIM_E '\x3'


void error(const char* msg);
int readArgs(int argc, char* argv[]);
int readRequest(char* request, char* cmd, char* fn, int* dataPort);

int startUpSocket(int portno);
int acceptConnection(int serverSocket, char* clientName);
int initConnection(char* hostname, int portno);

int safeRead(int sockfd, char* msg, int msglen);
int safeWrite(int sockfd, char* msg);
void safeWriteFile(int sockfd, FILE* file);

#endif /* FTSEVER_UTILS_H_ */