/*
 * parser_util.c 
 */ 

#include "parser_internal.h"

void parser_incorrect_token(char tok1, char tok2, int buf_ptr, char *buf) {
    char error[30];
    int start = (buf_ptr < 10) ? 0 : buf_ptr - 10;
    int end = (buf_ptr > PARSE_BUF_SIZE - 20) ? PARSE_BUF_SIZE : buf_ptr + 20;

    memset(error, '\0', sizeof error);
    memcpy(error, &buf[start], end - start);

    log_file(LogMessage, "Parser", "Buffer: %s", error);
    log_file(LogMessage, "Parser", "        " "        ^");

    if (tok1 < 'a') {
        log_file(LogError, "Parser", "Couldn't match token %d to token %c", tok1, tok2);
    } else {
        log_file(LogError, "Parser", "Couldn't match token %c to token %c", tok1, tok2);
    }
}

int parser_get_char(int socket_client, int *buf_ptr, char *buf, char *c) {
    if (parser_get_message(socket_client, buf_ptr, buf) != SERVER_MESSAGE) {
        log_file(LogWarn, "Parser", "Tried to get char, but didn't have any remaining");
        return -1;
    }

    *c = buf[(*buf_ptr)++];
    return 0;
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

