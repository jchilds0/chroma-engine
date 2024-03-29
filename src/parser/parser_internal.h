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
#define LOG_PARSER          1

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

    /* hub */ 
    STRING, 
    INT,
} Token;

int             parser_tcp_timeout_listen(int server_sock);
ServerResponse  parser_tcp_recieve_message(int socket_client, char *buf);

char            parser_get_char(int socket_client, int *buf_ptr, char *buf);
void            parser_clean_buffer(int *buf_ptr, char *buf);
ServerResponse  parser_get_message(int socket_client, int *buf_ptr, char *buf);

#endif // !PARSER_INTERNAL

