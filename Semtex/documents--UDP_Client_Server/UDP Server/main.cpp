#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <fcntl.h>
#include <sys\types.h>
#include <winsock2.h>
#include <WS2tcpip.h>


#define MYBUFSIZE 512
#define MAX_NUM_THREADS 2
static const int MAX_PENDING = 5;		// Maximum outstanding connecton server
using namespace std;
HANDLE m_thread_Handle = 0;
HANDLE m_thread_Handle_2 = 0;
HANDLE m_threadArray[2];
// threads for our server
DWORD thread[2];
DWORD WINAPI receive_cmds(LPVOID);


typedef struct _SOCKET_INFROMATION {
	CHAR Buffer[8192];
	WSABUF DataBuf;
	SOCKET socket;
	OVERLAPPED Overlap;
	DWORD BytesSend;
	DWORD BytesRECV;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

fd_set ReadSet, WriteSet;
DWORD total;
DWORD RecvBytes, SendBytes;
DWORD Flags;

// Prototypes
BOOL CreateSocketInformation(SOCKET);
void FreeSocketInformation(DWORD);

// Global var
DWORD TotalSockets = 0;
LPSOCKET_INFORMATION SocketArray[FD_SETSIZE];

void shutdown_close(SOCKET s)
{
	// Tell OS socket is about to close
	shutdown(s,SD_SEND);

	closesocket(s);
}

void shutdown_close_and_exit(SOCKET s)
{
	shutdown_close(s);
	bool handler;
	for (int i = 0; i < MAX_NUM_THREADS; i++)
	{
		handler = CloseHandle(m_threadArray[i]);	

		if (!handler)
		{
			// simply prints out error message based on the current handle
			if (i == 0)
				printf("Failed to close the first handle.\n");
			else if (i == 1)
				printf("Failed to close the second handle.\n");
		}
		else if (handler)
		{
			// same as above but a success message
			if (i == 0)
				printf("Successfully closed the first handle.\n");
			else if (i == 1)
				printf("Successfully closed the second handle.\n");
		}
	}
	exit(1);
}

void HandleTCPClient(SOCKET clnt_socket)
{
	/*
	char buffer[MYBUFSIZE]; // buffer for echo string

	// Receive message from client
	int numBytesRcvd = recv(clnt_socket, buffer, MYBUFSIZE, 0);

	Sleep(1000);
	
	if(numBytesRcvd < 0)
	{
		printf("recv() failed\n");
		printf("Terminating thread\n");
		shutdown_close_and_exit(clnt_socket);
	}
	
	// send the received string and receive again until end of stream
	while (numBytesRcvd > 0) // 0 is end of stream
	{
		// echo message to client
		int numBytesSent = send(clnt_socket, buffer, numBytesRcvd, 0);

		Sleep(1000);
		if(numBytesSent < 0)
		{
			printf("send() failed\n");
			printf("Terminating thread\n");
			shutdown_close_and_exit(clnt_socket);
		}
		else if(numBytesSent != numBytesRcvd)
		{
			printf("send(), sent unexpected number of bytes");
			printf("Terminating thread\n");
			shutdown_close_and_exit(clnt_socket);
		}

		// check for if any more data to receive
		numBytesRcvd = recv(clnt_socket, buffer, MYBUFSIZE, 0);
		Sleep(1000);
		if(numBytesRcvd < 0)
		{
			printf("2nd recv() check failed");
			printf("Terminating thread\n");
			shutdown_close_and_exit(clnt_socket);
		}
	}
	*/
	// Check each socket for Read and Write notification until the number
			// of sockets in Total is satisfied
			for (int i = 0; total > 0 && i < TotalSockets; i++)
			{
				LPSOCKET_INFORMATION SocketInfo = SocketArray[i];

				if (FD_ISSET(SocketInfo->socket, &ReadSet) )
				{
					printf("Read set extracting.\n");
					total--;

					SocketInfo->DataBuf.buf = SocketInfo->Buffer;
					SocketInfo->DataBuf.len = MYBUFSIZE;

					if (WSARecv(SocketInfo->socket, &(SocketInfo->DataBuf), 1,
						&RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							printf("WSARecv() failed %d\n", WSAGetLastError() );
							FreeSocketInformation(i);
						}
						continue;
					}
					else
					{
						printf("Received message.\n");
						SocketInfo->BytesRECV = RecvBytes;

						if (RecvBytes == 0)
						{
							printf("Stopping. No more received bytes.\n");
							FreeSocketInformation(i);
							continue;
						}
					}
				}

				// IF the write set has been set, extract the data
				if (FD_ISSET(SocketInfo->socket, &WriteSet) )
				{
					printf("Write Set extracting...\n");
					total--;

					SocketInfo->DataBuf.buf = SocketInfo->Buffer + SocketInfo->BytesSend;
					SocketInfo->DataBuf.len = SocketInfo->BytesRECV - SocketInfo->BytesSend;


					if (WSASend(SocketInfo->socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							printf("WSASend() failed %d\n", WSAGetLastError() );
							FreeSocketInformation(i);
						}
						
						continue;
					}
					else
					{
						printf("Sending message.\n");
						SocketInfo->BytesSend += SendBytes;

						if (SocketInfo->BytesSend == SocketInfo->BytesRECV)
						{
							printf("Resetting sent and received bytes.\n");
							SocketInfo->BytesSend = 0;
							SocketInfo->BytesRECV = 0;
					
						}
					}
				}
			}

	printf("Closing Thread.\n");
	m_threadArray[0] = m_thread_Handle;	// set the first thread handle automatically

// if the second handle isn't NULL, then set the second handle array
if (m_thread_Handle_2 != NULL)
{
	printf("Setting the second Handle array.\n");
	m_threadArray[1] = m_thread_Handle_2;
}
// Now if the second handle array isn't NULL then wiat on both threads for ten seconds
if (m_threadArray[1] != NULL)
{
	printf("Waiting on two threads.\n");
	WaitForMultipleObjects(2, m_threadArray, TRUE, 10000);
}
// otherwise if it is NULL, then wait only on the first thread for the same amount of time
else 
{
	printf("Waiting only on a single thread.\n");
	WaitForMultipleObjects(1, m_threadArray, TRUE, 10000);
}
	shutdown_close_and_exit(clnt_socket);
	
}


int main( int argc, char * argv[] )
{
	u_short server_port;

	u_long NonBlock;

	SOCKET previousSock = INVALID_SOCKET;	// keeps track of a previous SOCKET
	SOCKET ListenSocket;
	SOCKET clntSock = INVALID_SOCKET;
	if (argc == 2 )
	{
		server_port = atoi(argv[1]);
		printf("Setting port number to: %d\n", server_port);
	}
	WSAData wsaData;
	
	// Init Winsock
	if(WSAStartup(MAKEWORD(2,0), &wsaData) != 0)
	{
		fprintf(stderr, "Error: WSAStartup() failed");
		exit(1);
	}

	//SOCKET server_sock;
	if ( (ListenSocket = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP) ) < 0)
	{
		printf("Socket error: %d\n", ListenSocket);
		shutdown_close_and_exit(ListenSocket);
	}

	sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr) );
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(server_port);

	if (bind(ListenSocket, (struct sockaddr*)&servAddr, sizeof(servAddr) ) < 0)
	{
		printf("bind() failed\n");
		shutdown_close_and_exit(ListenSocket);
	}

	if (listen(ListenSocket, MAX_PENDING) < 0)
	{
		printf("listen() failed\n");
		shutdown_close_and_exit(ListenSocket);
	}
	 
#pragma region Myregion
	while (true)
	{
		// Prepare the Read and Write socket sets for network I/O notficiation
		FD_ZERO(&ReadSet);
		FD_ZERO(&WriteSet);

		// Watch out for connection attempts
		FD_SET(ListenSocket, &ReadSet);

		// Set the read and write notification for each socket based on the 
		// current state buffer. If there is data remaining in the buffer
		// then set the Write set otherwise set the Read set.
		for (int i = 0; i < TotalSockets; i++)
			if (SocketArray[i]->BytesRECV > SocketArray[i]->BytesSend)
				FD_SET(SocketArray[i]->socket, &WriteSet);
			else 
				FD_SET(SocketArray[i]->socket, &ReadSet);

			if ((total = select(0, &ReadSet, &WriteSet, NULL, NULL)) == SOCKET_ERROR)
			{
				printf("select() failed %d\n", WSAGetLastError() );
				shutdown_close_and_exit(ListenSocket);
			}
			

			// Check for arriving connections on the server socket
			if (FD_ISSET(ListenSocket, &ReadSet) )
			{
				total--;
				struct sockaddr_in clntAddr;
				socklen_t cltnAddrLen = sizeof(clntAddr);
				if ( (clntSock = accept(ListenSocket, (struct sockaddr*) &clntAddr, &cltnAddrLen) ) != INVALID_SOCKET)
				{
					printf("Client Sock accepted.\n");
					
				}
				
				else if (clntSock < 0)
				{
					printf("accept() failed\n");
					shutdown_close_and_exit(clntSock);
				}



#pragma region PrintCleint
	char clntName[INET_ADDRSTRLEN];
		if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName,
			sizeof(clntName) ) != NULL) 
		{
			printf("Handling client: %s%d\n", clntName, ntohs(clntAddr.sin_port) );
			// handler of the thread process
			if (m_thread_Handle == 0)
			{
				m_thread_Handle = CreateThread(NULL, 0, receive_cmds, (LPVOID)clntSock, 0, &thread[0]);
			
				if (m_thread_Handle == 0)
				{
					ExitProcess(clntSock);
				}
				else
				{
					printf("Successfully created thread\n");
					previousSock = clntSock;	// set the previousSock value to the first clntSock

					if (CreateSocketInformation(clntSock) == FALSE)
					{
						printf("CreateSocketInformation() failed %d\n", WSAGetLastError() );
						shutdown_close_and_exit(clntSock);
					}
				}
			}
		
		if ( (m_thread_Handle != 0) && (m_thread_Handle_2 == 0) && (clntSock != previousSock) )
		{
			// Create a second thread
			m_thread_Handle_2 = CreateThread(NULL, 0, receive_cmds, (LPVOID)clntSock, 0, &thread[1]);

			if (m_thread_Handle_2 == 0)
			{
				ExitProcess(clntSock);
			}
			else
			{
				printf("Successfully created a second thread\n");

				if (CreateSocketInformation(clntSock) == FALSE)
				{
					printf("CreateSocketInformation() failed %d\n", WSAGetLastError() );
					shutdown_close_and_exit(clntSock);
				}
			}
		}
	
		}
		else
			puts("Unable to get client address\n");
#pragma endregion
			}
			
	
/*
		struct sockaddr_in clntAddr;
		
		socklen_t cltnAddrLen = sizeof(clntAddr);
		clntSock = accept(server_sock, (struct sockaddr*) &clntAddr, &cltnAddrLen);
		if (clntSock < 0)
		{
			printf("accept() failed\n");
			shutdown_close_and_exit(clntSock);
		}

		char clntName[INET_ADDRSTRLEN];
		if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName,
			sizeof(clntName) ) != NULL) 
		{
			printf("Handling client: %s%d\n", clntName, ntohs(clntAddr.sin_port) );
			// handler of the thread process
		if (m_thread_Handle == 0)
		{
			m_thread_Handle = CreateThread(NULL, 0, receive_cmds, (LPVOID)clntSock, 0, &thread[0]);
			
			if (m_thread_Handle == 0)
			{
				ExitProcess(clntSock);
			}
			else
			{
				printf("Successfully created thread\n");
				previousSock = clntSock;	// set the previousSock value to the first clntSock
			}
		}
		
		if ( (m_thread_Handle != 0) && (m_thread_Handle_2 == 0) && (clntSock != previousSock) )
		{
			// Create a second thread
			m_thread_Handle_2 = CreateThread(NULL, 0, receive_cmds, (LPVOID)clntSock, 0, &thread[1]);

			if (m_thread_Handle_2 == 0)
			{
				ExitProcess(clntSock);
			}
			else
			{
				printf("Successfully created a second thread\n");
			}
		}
	
		}
		else
			puts("Unable to get client address\n");
	
		//HandleTCPClient(clntSock);
*/

	}	// end of infinite for loop
	
#pragma endregion

	
}

// The main thread that receives and sends commands to and from the client
DWORD WINAPI receive_cmds(LPVOID lpParam)
{
	// create a Socket and set it equal to the void parameter value
	SOCKET m_curr_clnt = (SOCKET)lpParam;

	// call the same function we used before 
	//but inside the thread once it's created
	HandleTCPClient(m_curr_clnt);	
	
	//printf("Did you get here?\n");
	return 1;
}

BOOL CreateSocketInformation(SOCKET sock)
{
	LPSOCKET_INFORMATION SI;

	if (( SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION)))
		== NULL)
	{
		printf("GlobalAlloc failed %d\n", GetLastError() );
		return FALSE;
	}

	SI->socket = sock;
	SI->BytesSend = 0;
	SI->BytesRECV = 0;

	
	SocketArray[TotalSockets] = SI;
	TotalSockets++;
	printf("Current number of sockets %d\n", TotalSockets);
	return TRUE;
}

void FreeSocketInformation(DWORD Index)
{
	LPSOCKET_INFORMATION SI;
	SI = SocketArray[Index];

	shutdown_close(SI->socket);
	printf("Closing socket number %d\n", SI->socket);
	GlobalFree(SI);

	// Squash the Socket array
	for (int i = Index; i < TotalSockets; i++)
	{
		SocketArray[i] = SocketArray[i+1];
	}

	TotalSockets--;
}

