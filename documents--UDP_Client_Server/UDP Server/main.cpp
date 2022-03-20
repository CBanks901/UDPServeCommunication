
#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>

#define MYBUFSIZE	128

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
    SOCKET srvr_socket;
    struct sockaddr_in srvr_addr;
    struct sockaddr_in clnt_addr; // Client address
    int addr_len = 0;
    WSADATA wsaData;
	char recv_buf[MYBUFSIZE];
	int recv_msg_len;
	short portnum;

   
	if (argc == 2)
	{
		portnum = atoi(argv[1]);
		printf("Setting port number to %d\n", portnum);

	} else
	{
		fprintf(stderr, "Usage: %s port_num_to_listen_at\n",
				argv[0]);
        exit(1);
	}

	// Init Winsock
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
    {
		fprintf(stderr, "Error: WSAStartup() failed");
        exit(1);
    }

    // Create UDP datagram socket for incoming connections
    if ((srvr_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		fprintf(stderr, "Error: socket() failed with error %d\n", WSAGetLastError());
		shutdown_close_and_exit(srvr_socket);
	}

    // Construct local address structure
    memset(&srvr_addr, 0, sizeof(srvr_addr));
    srvr_addr.sin_family = AF_INET;
    srvr_addr.sin_addr.s_addr = INADDR_ANY;  // Accept messages on any network interface.
    srvr_addr.sin_port = htons(portnum);	 // Bind the port number specified on the command line.

    /* Bind to the local address */
    if (bind(srvr_socket, (struct sockaddr *) &srvr_addr, sizeof(srvr_addr)) == SOCKET_ERROR)
	{
		fprintf(stderr, "Error: socket() failed with error %d", WSAGetLastError());
	    shutdown_close_and_exit(srvr_socket);
	}

	// Loop forever
    for (;;)
    {
        // Set the size of the in-out parameter, where the client address
		// and port number will be stored by the OS
        addr_len = sizeof(clnt_addr);

		// Receive message from client
		if ((recv_msg_len = recvfrom(srvr_socket, recv_buf, MYBUFSIZE, 0,
			(struct sockaddr *) &clnt_addr, &addr_len )) == SOCKET_ERROR) {

			fprintf(stderr, "Error: recvfrom() failed with error %d\n", WSAGetLastError());
			shutdown_close_and_exit(srvr_socket);
		} else {
			printf("Received message of size %d: %s\n from client %s:%d\n", 
					recv_msg_len, recv_buf, inet_ntoa(clnt_addr.sin_addr),
					ntohs(clnt_addr.sin_port));
		}

		
		// 'echo' message back to client
		if ((recv_msg_len = sendto(srvr_socket, recv_buf, recv_msg_len, 0,
			(struct sockaddr *) &clnt_addr, addr_len )) == SOCKET_ERROR) {

			fprintf(stderr, "Error: sendto() failed with error %d\n", WSAGetLastError());
			shutdown_close_and_exit(srvr_socket);
		} else {
			printf("Echo'd message %s to client %s:%d\n", 
					recv_buf, inet_ntoa(clnt_addr.sin_addr),
					ntohs(clnt_addr.sin_port));
		}
    }
}
