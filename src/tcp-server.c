/*
 * tcp_server.c 
 */

#include "chroma-engine.h"
#include <asm-generic/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int start_tcp_server(char *addr, int port) {
    int socket_desc;
    struct sockaddr_in server_addr;

    // create socket 
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0) {
        printf("Error while creating socket\n");
        return -1;
    }

    printf("Socket created successfully\n");

    // set port and ip 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(addr);

    // bind to the set port and ip 
    if (bind(socket_desc, (struct sockaddr*) &server_addr, sizeof server_addr) < 0) {
        printf("Couldn't bind to the port\n");
        return -1;
    }

    printf("Done with binding\n");

    return socket_desc;
}

int listen_for_client(int server_sock) {
    int client_sock, client_size;
    struct sockaddr_in client_addr;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000 / FRAMERATE;

    setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    // listen for clients 
    if (listen(server_sock, 1) < 0) {
        printf("Error while listening\n");
        return TIMEOUT;
    }

    // printf("Listening for incoming connection....\n");

    client_size = sizeof client_addr;
    client_sock = accept(server_sock, (struct sockaddr*) &client_addr, &client_size);

    if (client_sock < 0) {
        printf("Can't accept\n");
        return TIMEOUT;
    }

    printf("Client connected at IP: %s and port %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    return client_sock;
}

int recieve_message(int client_sock, char *client_message) {
    char server_message[2000];

    // clean buffers 
    memset(server_message, '\0', sizeof server_message);

    // recieve clients message 
    if (recv(client_sock, client_message, sizeof client_message, 0) < 0) {
        printf("Couldn't recieve\n");
        return TIMEOUT;
    }

    if (client_message[0] == 4) {
        printf("Connection closed\n");
        return CLOSE_SOCKET;
    }

    // printf("Msg from client: %s\n", client_message);

    // respond to client 
    strcpy(server_message, "Recieved");

    if (send(client_sock, server_message, strlen(server_message), 0) < 0) {
        printf("Can't send\n");
        return TIMEOUT;
    }

    return 1;
}

