/*
 * Header for parser submodule
 */

#include "chroma-typedefs.h"

// ServerResponse MUST BE < 0 otherwise socket_client in parser will be incorrect
typedef enum {
    SERVER_MESSAGE = -3,
    SERVER_TIMEOUT,
    SERVER_CLOSE,
} ServerResponse;

typedef enum {
    VERSION = 0,
    LENGTH,
    ACTION,
    TEMPID,
    ATTR,
    VALUE,
    EOM,
} Token;

int parser_client_listen(int server_sock);
ServerResponse parser_tcp_recieve_message(int socket_client, char *buf);

ServerResponse parser_message(int socket_client);
char parser_get_char(int socket_client);
