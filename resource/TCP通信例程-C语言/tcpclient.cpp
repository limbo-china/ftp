#include <stdio.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
void main(int argc, char *argv[]) {

    //initialize Winsock.
    WSADATA wsaData;
    int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
    if ( iResult != NO_ERROR )
        printf("Error at WSAStartup()\n");

    //create the client socket
    SOCKET client;
    client = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if ( client == INVALID_SOCKET ) {
        printf( "Error at socket(): %ld\n", WSAGetLastError() );
        WSACleanup();
        return;
    }

    //set up the connection
    sockaddr_in clientService;

    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr( argv[1] );
    clientService.sin_port = htons(atoi(argv[2]));

    if ( connect( client, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) {
        printf( "Failed to connect.\n" );
        WSACleanup();
        return;
    }

    //send and recieve data
    int bytesSent;
    int bytesRecv = SOCKET_ERROR;
    char sendbuf[200] = "";
    char recvbuf[200] = "";

    gets(sendbuf);
    bytesSent = send( client, sendbuf, strlen(sendbuf), 0 );

    while( bytesRecv == SOCKET_ERROR ) {
        bytesRecv = recv( client, recvbuf, 32, 0 );

        if ( bytesRecv == 0 || bytesRecv == WSAECONNRESET ) {
            printf( "Connection Closed.\n");
            break;
        }
        if (bytesRecv < 0)
            return;
        printf( "Recv %ld bytes:%s\n", bytesRecv, recvbuf );
    }

    closesocket(client);
    return;
}
