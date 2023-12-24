/*
 * parser_internal.h
 *
 * Header file for the parser module source 
 * code. Should not be included by any source 
 * files outside the /parser dir.
 *
 */

#ifndef PARSER_INTERNAL
#define PARSER_INTERNAL

#define PARSE_BUF_SIZE      100
#define LOG_PARSER          0

// ServerResponse MUST BE < 0 otherwise socket_client in parser will be incorrect
typedef enum {
    SERVER_MESSAGE = -3,
    SERVER_TIMEOUT,
    SERVER_CLOSE,
} ServerResponse;

typedef enum {
    /* header tokens */
    VERSION = 0,
    LAYER,
    ACTION,
    TEMPID,

    /* other tokens */
    ATTR,
    VALUE,
    EOM,
} Token;

int            parse_client_listen(int server_sock);
ServerResponse parse_tcp_recieve_message(int socket_client, char *buf);

#endif // !PARSER_INTERNAL

