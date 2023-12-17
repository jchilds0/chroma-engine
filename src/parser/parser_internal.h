/*
 *
 */

#ifndef PARSER_INTERNAL
#define PARSER_INTERNAL

#define MAX_BUF_SIZE        512
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
    LENGTH,
    ACTION,
    TEMPID,

    /* other tokens */
    ATTR,
    VALUE,
    EOM,
} Token;

int parse_client_listen(int server_sock);
ServerResponse parse_tcp_recieve_message(int socket_client, char *buf);

ServerResponse parse_message(int socket_client);
char parse_get_char(int socket_client);

#endif // !PARSER_INTERNAL

