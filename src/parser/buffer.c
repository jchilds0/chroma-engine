/*
 * Buffer for parser
 */

#include "chroma-engine.h"
#include "chroma-typedefs.h"
#include "parser.h"
#include <string.h>

static char buf[MAX_BUF_SIZE];
static int buf_ptr = 0;

char parser_get_char(int socket_client) {
    parser_message(socket_client);
    return buf[buf_ptr++];
}

ServerResponse parser_message(int socket_client) {
    if (buf[buf_ptr] != '\0') {
        return SERVER_MESSAGE;
    }

    buf_ptr = 0;
    memset(buf, '\0', MAX_BUF_SIZE);
    return parser_tcp_recieve_message(socket_client, buf);
}

