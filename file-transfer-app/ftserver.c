/*********************************************************************
** ftserver.c
** Description: Server side of client-server file transfer application
*********************************************************************/

#include "ftserver-utils.h"


/*********************************************************************
** MAIN **/

int main(int argc, char *argv[]) {
	int serverSocket, controlSocket, dataSocket;
	int dataPort, controlPort, cpid, cstatus, n;
	char clientName[NAME_LENGTH],cmd[NAME_LENGTH], fn[NAME_LENGTH], fntemp[NAME_LENGTH];
	char buffer[BUFFER_LENGTH], cmdtemp[BUFFER_LENGTH];
	FILE* file;

	/* Configure and start up socket */
	controlPort = readArgs(argc, argv);
	serverSocket = startUpSocket(controlPort);
	listen(serverSocket, QUEUE_SIZE);
	printf("Server open on port %s.\n", argv[1]);
	
	/* Start accepting new connections */
	while (1) {
		/* Clean up zombie children */
		do {
			cpid = waitpid(-1, &cstatus, WNOHANG);
		} while (cpid > 0);

		/* Accept new connection */
		controlSocket = acceptConnection(serverSocket, clientName);

		/* Fork child to handle new connection */
		pid_t spawnpid = fork();
		if (spawnpid == -1) {
			error("Forking");
		}

		/* In parent: Move on */
		if (spawnpid > 0) {
			continue;
		}

		/* In child: Continue handling new connection */

		/* Receive client's cmd. If invalid, send error message */
	 	safeRead(controlSocket, buffer, BUFFER_LENGTH);
	 	if (readRequest(buffer, cmd, fn, &dataPort) < 0) {
			safeWrite(controlSocket, MSG_INVALID_CMD);

		/* Valid command. Respond to "-l" */
		} else if (strcmp(cmd, "-l") == 0) {
			printf("List directory requested on port %d.\n", dataPort);
			
			/* Write dir structure to hidden file .temp[pid] */
			sprintf(fntemp, ".temp%d", getpid());
			sprintf(cmdtemp, "ls > %s", fntemp);
			system(cmdtemp);

			/* Open .temp[pid] */
			file = fopen(fntemp, "r");
			if (file == NULL) {
				error("Opening file");
			
			/* All OK */
			} else {
				/* Send confirmation to client */
				safeWrite(controlSocket, MSG_OK);

				/* Wait for client's confirmation that data socket was set up */
				safeRead(controlSocket, buffer, BUFFER_LENGTH);

				/* Initiate data connection and send directory structure */
				if (strcmp(buffer, MSG_OK) == 0) {
					dataSocket = initConnection(clientName, dataPort);
					safeWriteFile(dataSocket, file);
					printf("Sending directory structure to %s:%d.\n", clientName, dataPort);
					close(dataSocket);					
				}

			}

			/* Clean up */
			fclose(file);
			sprintf(cmdtemp, "rm %s", fntemp);
			system(cmdtemp);

		/* Valid command. Respond to "-g" */
		} else if (strcmp(cmd, "-g") == 0) {
			printf("File '%s' requested on port %d.\n", fn, dataPort);

			/* Open file. Send error msg if error */
			file = fopen(fn, "r");
			if (file == NULL) {
				safeWrite(controlSocket, MSG_FILE_NOT_FOUND);
			
			/* All OK */
			} else {
				/* Send confirmation to client */ 
				safeWrite(controlSocket, MSG_OK);

				/* Wait for client's confirmation that data socket was set up */
				safeRead(controlSocket, buffer, BUFFER_LENGTH);

				/* Initiate data connection and send file content */
				if (strcmp(buffer, MSG_OK) == 0) {				
					dataSocket = initConnection(clientName, dataPort);
					printf("Sending '%s' to %s:%d.\n", fn, clientName, dataPort);
					safeWriteFile(dataSocket, file);
					close(dataSocket);
				}
			}

			fclose(file);
		}

		/* Clean up */
		close(controlSocket);
		break;
	}

	/* Clean up */	
	close(serverSocket);
	exit(1);
}
