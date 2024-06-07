/*
 * parser_tcp.c 
 *
 * Functions for interacting with the tcp 
 * server for the engine, which recieves 
 * graphics requests from Chroma-Viz.
 *
 */

#include "chroma-engine.h"
#include "log.h"
#include "parser_internal.h"
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
            log_file(LogError, "Parser", "Too many failed attemps to create a socket");
        }

        if (socket_desc < 0) {
            log_file(LogWarn, "Parser", "Error while creating socket, trying again...");
            attempts++;
            sleep(1);
            continue;
        }
    }

    log_file(LogMessage, "Parser", "Socket created successfully");

    // set port and ip 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(addr);

    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    // bind to the set port and ip 
    attempts = 0;
    while (bind_soc < 0) {
        bind_soc = bind(socket_desc, (struct sockaddr*) &server_addr, sizeof server_addr); 

        if (attempts++ > MAX_ATTEMPTS) {
            log_file(LogError, "Parser", "Too many attemps to bind socket at addr %s to port %d", addr, port);
        }

        if (bind_soc < 0) {
            log_file(LogWarn, "Parser", "Couldn't bind to the port, trying again...");
            sleep(1);
        }
    }

    log_file(LogMessage, "Parser", "Done with binding");

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100 / CHROMA_FRAMERATE;

    setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    // listen for clients 
    if (listen(socket_desc, MAX_CONNECTIONS) < 0) {
        //log_file(LogWarn, "Parser", "Error while listening");
        return SERVER_TIMEOUT;
    }

    log_file(LogMessage, "Parser", "Listening for incoming connection....");

    return socket_desc;
}

int parser_tcp_start_client(char *addr, int port) {
    int socket_desc;
    struct sockaddr_in server_addr;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0) {
        log_file(LogError, "Parser", "Unable to create socket");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(addr);

    if (connect(socket_desc, (struct sockaddr *)&server_addr, sizeof server_addr) < 0) {
        log_file(LogError, "Parser", "Unable to connect to server");
        return -1;
    }

    return socket_desc;
}

ServerResponse parser_tcp_recieve_message(int client_sock, char *client_message) {
    char server_message[PARSE_BUF_SIZE];

    // clean buffers 
    memset(server_message, '\0', sizeof server_message);

    // recieve clients message 
    if (recv(client_sock, client_message, PARSE_BUF_SIZE, 0) < 0) {
        return SERVER_TIMEOUT;
    }

    if (LOG_PARSER) {
        log_file(LogMessage, "Parser", "Recieved %s", client_message);
    }

    // respond to client 
    //strcpy(server_message, "Recieved");

    if (send(client_sock, server_message, strlen(server_message), 0) < 0) {
        printf("Can't send\n");
        return SERVER_TIMEOUT;
    }

    if (client_message[0] == END_OF_CONN) {
        log_file(LogMessage, "Parser", "Client closed connection");
        return SERVER_CLOSE;
    }

    return SERVER_MESSAGE;
}

