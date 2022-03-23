#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <sys\types.h>

#define BUFSIZE 512  /* Size of receive buffer */
void shutdown_close(SOCKET s)
{
	// Tell the operating system the socket is
	// about to be closed	
	shutdown(s, SD_SEND);	

	// close the socket….
	closesocket(s); 
}

void shutdown_close_and_exit(SOCKET s)
{
	shutdown_close(s);
	WSACleanup();
	exit(1);
}

void main(int argc, char *argv[])
{
   // SOCKET servSock;
  //  struct sockaddr_in srvr_addr;
  //   struct sockaddr_in recv_addr;

	
    int addr_len = 0; 
    WSADATA wsaData;
	char mesg_buf[BUFSIZE];
	char *mesg_to_send;
	long server_IP;
	short portnum;
	int iResult;

	
	if (argc == 4)
	{
		portnum = atoi(argv[1]);
		printf("Setting port number to %d\n", portnum);

		server_IP = inet_addr(argv[2]);
		printf("Target server IP address is %s\n", argv[2]);
		
		mesg_to_send = argv[3];
	} else
	{
		fprintf(stderr, "Usage: %s server_port_num server_IP_address message_to_send\n",
				argv[0]);
        exit(1);
	}
   
	// Init Winsock
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
    {
		fprintf(stderr, "Error: WSAStartup() failed with error %d\n", WSAGetLastError());
        exit(1);
    }
	

  /* // Create socket for incoming connections
    if ((servSock = socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		fprintf(stderr, "Error: socket() failed with error %d\n", WSAGetLastError());
		shutdown_close_and_exit(servSock);
	}*/

		// DELETE THIS------------------------------------------------------------
#pragma region SocketCreation

	struct addrinfo* result = NULL,
		*ptr = NULL,
		hints;
	
	//memset(&hints, 0, sizeof(hints) );
	ZeroMemory(&hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	char *addrString = argv[1];
	char *portString = argv[2];
	//Resolve the server address and port
	//iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	iResult = getaddrinfo(addrString, portString, &hints, &result);
	
	if (iResult != 0)
	{
		printf("getaddrinfo failed %d\n", iResult);
		WSACleanup();
		return;
	}

	// Create a socket object 
	SOCKET ConnectSocket = INVALID_SOCKET;

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	//Create the socket for connceting to the server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	// Check the socket for important errors
	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError() );
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

#pragma endregion

#pragma region Connecting Sockets
iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

if (iResult == SOCKET_ERROR)
{
	closesocket(ConnectSocket);
	ConnectSocket = INVALID_SOCKET;
}

// Free the resources returned by getaddrinfo and print error message
if (ConnectSocket == INVALID_SOCKET)
{
	printf("Unable to connect to Server!\n");
	WSACleanup();
	return;
}

#pragma endregion
   /* // Construct local address structure
	// with server address.  This is like
	// addressing the envelope of a letter.
    memset(&srvr_addr, 0, sizeof(srvr_addr));
    srvr_addr.sin_family = AF_INET;
    srvr_addr.sin_addr.s_addr = server_IP;
    srvr_addr.sin_port = htons(portnum);

    // Set the size of the in-out parameter
    addr_len = sizeof(recv_addr);

	if (sendto(servSock, mesg_to_send, strlen(mesg_to_send) + 1, 0,
				(sockaddr *) &srvr_addr, addr_len) == SOCKET_ERROR)
	{
		fprintf(stderr, "Error: sendto() failed with error %d\n", WSAGetLastError());
		shutdown_close_and_exit(servSock);
	}
	
	printf("Send message %s to server at %s:%d\n", argv[3], argv[2], portnum);
	*/
	
		 // DELETE THIS------------------------------------------------------------
#pragma region Sending and Receiving Data on the Client
	int recvbuflen = BUFSIZE;

	//int myNum = 123;
	//unsigned NetInt;
	UINT32 myNum = 1;
	myNum = htonl(myNum);

	//NetInt =htonl( (unsigned) myNum);

	// Send an initial buffer
	//iResult = send(ConnectSocket, mesg_to_send, (int) strlen(mesg_to_send), 0);
	iResult = send(ConnectSocket, (char*)&myNum, sizeof(myNum), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed: %d\n", WSAGetLastError() );
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	printf("Bytes Sent: %ld\n", iResult);
	//printf("Message sent: %ld\n", &myNum);

	// Sleep 1 full second to allow message  to get to server and be sent back...
	Sleep(1000);
	//addr_len = sizeof(recv_addr);

	// shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data

	//---------------------- Unhighlight this later -------------------//
	/*
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("Shutdown failed: %d\n", WSAGetLastError() );
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}
	*/

	// Received data until the server closes the connection
	do {
		//iResult = recv(ConnectSocket, mesg_buf, recvbuflen, 0);
		iResult = recv(ConnectSocket, (char*)&myNum, sizeof(long), 0);
		myNum = ntohl(myNum);
		if (iResult > 0)
		{
			printf("Bytes received: %d\n", iResult);
			//printf("Received message: %s\n", mesg_buf);
			printf("Received message: %d\n", myNum);
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed Client: %d\n", WSAGetLastError() );

	} while (iResult > 0);

#pragma endregion
	
#pragma region Disconnection
// shutdown the second half of the connection since no more data will be sent 
	shutdown_close(ConnectSocket);
#pragma endregion


}

