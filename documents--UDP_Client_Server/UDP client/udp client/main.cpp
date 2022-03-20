
#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>

#define BUFSIZE 128  /* Size of receive buffer */


void shutdown_close(SOCKET s)
{

	// Tell the operating system the socket is
	// about to be closed	
	shutdown(s, SD_BOTH); 

	// close the socket….
	closesocket(s); 
}

void shutdown_close_and_exit(SOCKET s)
{
	shutdown_close(s);
	exit(1);
}

void main(int argc, char *argv[])
{
    SOCKET servSock;
    struct sockaddr_in srvr_addr;
    struct sockaddr_in recv_addr;
    int addr_len = 0; 
    WSADATA wsaData;
	char mesg_buf[BUFSIZE];
	char *mesg_to_send;
	long server_IP;
	short portnum;


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

    // Create socket for incoming connections
    if ((servSock = socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		fprintf(stderr, "Error: socket() failed with error %d\n", WSAGetLastError());
		shutdown_close_and_exit(servSock);
	}

    // Construct local address structure
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

	// Sleep 1 full second to allow message  to get to server and be sent back...
	Sleep(1000);

	addr_len = sizeof(recv_addr);

	if (recvfrom(servSock, mesg_buf, BUFSIZE, 0,
				(sockaddr *) &recv_addr, &addr_len) == SOCKET_ERROR)
	{
		fprintf(stderr, "Error: recvfrom() failed with error %d\n", WSAGetLastError());
		shutdown_close_and_exit(servSock);
	}

	printf("Received message %s from %s:%d\n", mesg_buf, 
			inet_ntoa(recv_addr.sin_addr), ntohs(recv_addr.sin_port));

	// close socket gracefully
	shutdown_close(servSock);

}
