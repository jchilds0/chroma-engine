/*
 * parser_http.c
 *
 */

#include "log.h"
#include "parser/parser_internal.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

void parser_http_get(int socket_client, const char *addr) {
    char msg[PARSE_BUF_SIZE];
    memset(msg, '\0', sizeof msg);

    sprintf(msg, "GET http://%s HTTP/1.1\nHost: ChromaViz\n\n", addr);
    if (send(socket_client, msg, strlen(msg), 0) < 0) {
        log_file(LogError, "Parser", "Error requesting graphics hub"); 
    }
}

void parser_http_header(int socket_client, int *buf_ptr, char *buf) {
    char line[PARSE_BUF_SIZE];
    int i;

    while (1) {
        i = 0;
        memset(line, '\0', sizeof line);

        while ((line[i++] = parser_get_char(socket_client, buf_ptr, buf)) != '\n');

        if (line[1] == '\n') {
            break;
        }

        line[i - 1] = '\0';
        if (LOG_PARSER) {
            log_file(LogMessage, "Parser", line);
        }
    }
}

