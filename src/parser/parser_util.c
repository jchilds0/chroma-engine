/*
 * parser_util.c 
 */ 

#include "log.h"
#include "parser_internal.h"
#include <string.h>

char parser_get_char(int socket_client, int *buf_ptr, char *buf) {
    if (parser_get_message(socket_client, buf_ptr, buf) != SERVER_MESSAGE) {
        log_file(LogWarn, "Parser", "Tried to get char, but didn't have any remaining");
    }

    return buf[(*buf_ptr)++];
}

void parser_clean_buffer(int *buf_ptr, char *buf) {
    *buf_ptr = 0;
    memset(buf, '\0', PARSE_BUF_SIZE);
}

ServerResponse parser_get_message(int socket_client, int *buf_ptr, char *buf) {
    if (*buf_ptr < PARSE_BUF_SIZE && buf[*buf_ptr] != '\0') {
        return SERVER_MESSAGE;
    }

    parser_clean_buffer(buf_ptr, buf);
    return parser_tcp_recieve_message(socket_client, buf);
}

