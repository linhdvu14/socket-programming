# *********************************************************************
# chatserve.py
# Description: Server side of client-server network chat application 
# *********************************************************************

from socket import *
from sys import *

BUFFER_SIZE = 501


# *********************************************************************
# startUp()
# Description: Gets command line args, starts up socket, get server's name
# Params: none
# Returns: server socket
# *********************************************************************
def startUp():
	# Check number of arguments
	if len(argv) != 2:
		print "Usage: python %s [port number]" % argv[0]
		exit(1)

	# Get server's handle name
	s_handle = raw_input("What's your name?> ")
	s_handle.rstrip()

	# Set server port number
	portno = int(argv[1])

	# Set up socket
	serverSocket = socket(AF_INET, SOCK_STREAM) 
	serverSocket.bind(('', portno)) 
	serverSocket.listen(1)
	print "Server ready. Listening for incoming connections."

	return serverSocket, s_handle


# *********************************************************************
# sendMessage()
# Description: Reads user input, send over connection
# Params:
#	connectionSocket: socket over which to exchange message
#	s_handle: server's name, to be prepended to outgoing msg
# Returns: 1 if server enters "/quit", 0 otherwise
# *********************************************************************
def sendMessage(connectionSocket, s_handle):
	# Get server's message 
	msg = raw_input(s_handle + "> ")
	msg.rstrip('\n')

	# If server quits, notify client
	if msg == "/quit":
		connectionSocket.send(msg)
		return 1

	# Write server's message to client
	connectionSocket.send(msg)
	return 0


# *********************************************************************
# receiveMessage()
# Description: Receives and displays client's message
# Params:
#	connectionSocket: socket over which to exchange message
#	c_handle: client's handle
# Returns: 1 if client sends "/quit", 0 otherwise
# *********************************************************************
def receiveMessage(connectionSocket, c_handle):
	# Read client's message
	msg = connectionSocket.recv(BUFFER_SIZE)
	msg.rstrip('\n')

	# If client quits, notify server
	if msg == "/quit":
		return 1

	# Display client's message
	print c_handle + "> " + msg
	return 0


# *********************************************************************
# ** MAIN **
def main():
	serverSocket, s_handle = startUp()

	# Listen for new connection
	while 1:
		connectionSocket, addr = serverSocket.accept()
		print "Connection established." 
		
		# Exchange handle info
		connectionSocket.send(s_handle)
		c_handle = connectionSocket.recv(BUFFER_SIZE)
		c_handle.rstrip('\n')

		# Exchange messages until either host quits
		while 1:	
			# Receive client's message
			if receiveMessage(connectionSocket, c_handle) == 1:
				break
		
			# Send server's message 
			if sendMessage(connectionSocket, s_handle) == 1:
				break
		
		# Either host has quit, close current connection
		connectionSocket.close()
		print "Connection closed."


if __name__ == '__main__':
	main()
