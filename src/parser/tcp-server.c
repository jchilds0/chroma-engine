/*
 * tcp_server.c 
 */

#include "chroma-engine.h"
#include "parser.h"
#include <arpa/inet.h>

#define MAX_ATTEMPTS        10

int parser_tcp_start_server(char *addr, int port) {
    int socket_desc = -1;
    int bind_soc = -1;
    int attempts = 0;
    struct sockaddr_in server_addr;

    // create socket 
    while (socket_desc < 0) {
        socket_desc = socket(AF_INET, SOCK_STREAM, 0);

        if (attempts++ > MAX_ATTEMPTS) {
            log_file(LogError, "Too many failed attemps to create a socket");
        }

        if (socket_desc < 0) {
            log_file(LogWarn, "Error while creating socket, trying again...");
            attempts++;
            sleep(1);
            continue;
        }
    }

    log_file(LogMessage, "Socket created successfully");

    // set port and ip 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(addr);

    // bind to the set port and ip 
    attempts = 0;
    while (bind_soc < 0) {
        bind_soc = bind(socket_desc, (struct sockaddr*) &server_addr, sizeof server_addr); 

        if (attempts++ > MAX_ATTEMPTS) {
            log_file(LogError, "Too many attemps to bind socket at addr %s to port %d", addr, port);
        }

        if (bind_soc < 0) {
            log_file(LogWarn, "Couldn't bind to the port, trying again...");
            sleep(1);
        }
    }

    log_file(LogMessage, "Done with binding");
    return socket_desc;
}

int parser_client_listen(int server_sock) {
    int client_sock;
    socklen_t client_size;
    struct sockaddr_in client_addr;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000 / CHROMA_FRAMERATE;

    setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    // listen for clients 
    if (listen(server_sock, 1) < 0) {
        log_file(LogWarn, "Error while listening");
        return SERVER_TIMEOUT;
    }

    //log_to_file(LogMessage, "Listening for incoming connection....");

    client_size = sizeof client_addr;
    client_sock = accept(server_sock, (struct sockaddr*) &client_addr, &client_size);

    if (client_sock < 0) {
        //log_file(LogWarn, "Can't accept connection");
        return SERVER_TIMEOUT;
    }

    log_file(LogMessage, "Client connected at IP: %s and port %i", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    return client_sock;
}

ServerResponse parser_tcp_recieve_message(int client_sock, char *client_message) {
    char server_message[MAX_BUF_SIZE];

    // clean buffers 
    memset(server_message, '\0', sizeof server_message);

    // recieve clients message 
    if (recv(client_sock, client_message, MAX_BUF_SIZE, 0) < 0) {
        //printf("Couldn't recieve\n");
        return SERVER_TIMEOUT;
    }

    //log_file(LogMessage, "Recieved %s", client_message);

    // respond to client 
    strcpy(server_message, "Recieved");

    if (send(client_sock, server_message, strlen(server_message), 0) < 0) {
        printf("Can't send\n");
        return SERVER_TIMEOUT;
    }

    return SERVER_MESSAGE;
}

