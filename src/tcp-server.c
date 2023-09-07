/*
 * tcp_server.c 
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void open_tcp_server(void) {
    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;
    char server_message[2000], client_message[2000];

    // clean buffers 
    memset(server_message, '\0', sizeof server_message);
    memset(client_message, '\0', sizeof client_message);
    
    // create socket 
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0) {
        printf("Error while creating socket\n");
        return;
    }

    printf("Socket created successfully\n");

    // set port and ip 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6100);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // bind to the set port and ip 
    if (bind(socket_desc, (struct sockaddr*) &server_addr, sizeof server_addr) < 0) {
        printf("Couldn't bind to the port\n");
        return;
    }

    printf("Done with binding\n");

    // listen for clients 
    if (listen(socket_desc, 1) < 0) {
        printf("Error while listening\n");
        return;
    }

    printf("Listening for incoming connection....\n");

    client_size = sizeof client_addr;
    client_sock = accept(socket_desc, (struct sockaddr*) &client_addr, &client_size);

    if (client_sock < 0) {
        printf("Can't accept\n");
        return;
    }

    printf("Client connected at IP: %s and port %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // recieve clients message 
    if (recv(client_sock, client_message, sizeof client_message, 0) < 0) {
        printf("Couldn't recieve\n");
        return;
    }

    printf("Msg from client: %s\n", client_message);

    // respond to client 
    strcpy(server_message, "This is the server's messge.");

    if (send(client_sock, server_message, strlen(server_message), 0) < 0) {
        printf("Can't send\n");
        return;
    }

    // close sockets 
    shutdown(client_sock, SHUT_RDWR);
    shutdown(socket_desc, SHUT_RDWR);
}
