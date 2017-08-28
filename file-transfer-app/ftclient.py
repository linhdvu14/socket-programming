# *********************************************************************
# ftclient.py
# Description: Client side of client-server file transfer application
# *********************************************************************

from socket import *
from sys import *
from os.path import *

BUFFER_LENGTH = 4096
MSG_OK = "OK"

DELIM_S = '\x02'
DELIM_E = '\x03'


# *********************************************************************
# readArgs()
# Description: Validates and reads command line args
# Params:
#	argv: passed command line args
# Returns: 
#	Server name
#	Server port
#	Command passed (-l or -g)
#	Data portno
#	String of passed request
# *********************************************************************
def readArgs(argv):
	# Validate command line args
	if (len(argv) != 5 and len(argv) != 6) or (argv[3] != "-l" and argv[3] != "-g") \
	or (len(argv) == 5 and argv[3] != "-l") or (len(argv) == 6 and argv[3] != "-g"):
		print "Usage: python %s [server name] [server portno] [-l / -g filename] [data portno]" % argv[0]
		exit(1)	

	# Read command line args
	serverName = argv[1]
	serverPort = int(argv[2])
	cmd = argv[3]
	dataPort = int(argv[len(argv) - 1])
	request = " ".join(argv[3:len(argv)])

	return serverName, serverPort, cmd, dataPort, request


# *********************************************************************
# initConnection()
# Description: Initiates connection with already running host. Exchanges
#	identity.
# Params:
#	argv: passed command line args
# Returns: 
#	Socket file descriptor
#	Hostname
# *********************************************************************
def initConnection(hostname, portno):
	# Start TCP control connection with server
	sockfd = socket(AF_INET, SOCK_STREAM) 
	sockfd.connect((hostname,portno))

	# Send client name
	clientName = gethostname()
	safeWrite(sockfd, clientName)

	# Receive server name
	serverName = safeRead(sockfd)
	print "New connection established with %s." % serverName

	return sockfd, serverName


# *********************************************************************
# startUpSocket()
# Description: Starts up socket for accepting connections
# Params:
#	portno: port number
# Returns: Socket file descriptor
# *********************************************************************
def startUpSocket(portno):
	sockfd = socket(AF_INET, SOCK_STREAM) 
	sockfd.bind(('', portno)) 
	sockfd.listen(1)
	return sockfd


# *********************************************************************
# safeRead()
# Description: Reads deliminated message from socket
# Params:
# 	sockfd: socket file descriptor
# Returns: Message read.
# *********************************************************************
def safeRead(sockfd):
	msg = ""

	# Look for start delim char
	while True:
		c = sockfd.recv(1)
		if c == DELIM_S:
			break

	# Start delim found. Read msg until end delim char reached
	while True:
		c = sockfd.recv(1)
		if (c == DELIM_E):
			break
		msg += c

	return msg


# *********************************************************************
# safeWrite()
# Description: Writes message to socket, prefixed with start delim char
#	and suffixed with end delim char. Ensures full message gets written
#	by looping until message size reached.
# Params:
# 	sockfd: socket file descriptor
# 	msg: message to be sent
# Returns: 0 if success, -1 if failure.
# *********************************************************************
def safeWrite(sockfd, msg):
	bytes_written = 0

	# Send start delim char
	sockfd.send(str(DELIM_S))

	# Send message
	while bytes_written != len(msg):
		n = sockfd.send(msg[bytes_written:(len(msg) - bytes_written)])
		if n < 0:
			return -1;
		bytes_written += n

	# Send end delim char
	sockfd.send(str(DELIM_E))

	return 0


# *********************************************************************
# ** MAIN **
def main():
	# Read command line args
	serverName, serverPort, cmd, dataPort, request = readArgs(argv)
	if cmd == "-g":
		fn = argv[4]
		fn_new = fn

	# Start control connection
	controlSocket, serverName = initConnection(serverName, serverPort)
	serverAddr = serverName + ":" + str(serverPort) 

	# Send request
	safeWrite(controlSocket, request)

	# Receive message
	msg = safeRead(controlSocket)

	# If error message, print and close connection
	if msg != MSG_OK:
		print msg + "\n"

	# If no error, start data connection, send confirmation over control connection
	# when data connection established
	else:
		dataSocket = startUpSocket(dataPort)
		safeWrite(controlSocket, MSG_OK)

		# If "-l" requested, read directory structure over data connection, dump to screen
		if cmd == "-l":
			dataSocket, addr = dataSocket.accept()
			print "Receiving directory structure from " + serverAddr + "."
			msg = safeRead(dataSocket)
			print msg

		# If "-g" requested, read file content over data connection, save to local file
		elif cmd == "-g":
			dataSocket, addr = dataSocket.accept()
			
			# If local file with duplicate name exists, ask user for new file name
			if exists(fn):
				fn_new = raw_input("File requested already existed. Enter new file name: ")
				fn_new.rstrip('\n')

			print "Receiving '" + fn + "' from " + serverAddr + "."
			file = open(fn_new, "w")
			msg = safeRead(dataSocket)
			file.write(msg)
			file.close()
			print "File transfer completed.\n"
		
		# Close data connection
		dataSocket.close()

	# Close control connection
	controlSocket.close()


if __name__ == '__main__':
	main()

