/*
 * Recieve graphics request over tcp and render to GtkDrawingArea
 */

#include "chroma-engine.h"
#include "parser.h"
#include <sys/socket.h>

static int socket_client = -1;

void parse_page(int *page_num, Action *action);
Token parser_get_token(char *value);

void parser_read_socket(int *page_num, Action *action) {
    if (socket_client < 0) {
        socket_client = parser_client_listen(engine.socket);
    } else {
        ServerResponse rec = parser_message(socket_client);

        switch (rec) {
        case SERVER_MESSAGE:
            parse_page(page_num, action);
            engine.hub->time = 0.0f;

            break;
        case SERVER_TIMEOUT:
            break;
        case SERVER_CLOSE:
            shutdown(socket_client, SHUT_RDWR);
            socket_client = -1;
            break;
        }
    }
}

void parse_page(int *page_num, Action *action) {
    char attr[MAX_BUF_SIZE];
    char value[MAX_BUF_SIZE];
    Token tok;
    int v_m, v_n, length;

    while ((tok = parser_get_token(attr)) != EOM
           && parser_get_token(value) != EOM) {
        switch (tok) {
            case VERSION:
                sscanf(value, "%d,%d", &v_m, &v_n);
                
                if (v_m != 1 && v_n != 2) {
                    log_file(LogError, "Incorrect encoding version v%d.%d, expected v1.2", v_m, v_n);
                }

                if (LOG_PARSER)
                    log_file(LogMessage, "Parsed v%d.%d", v_m, v_n); 
                break;
            case LENGTH:
                sscanf(value, "%d", &length);
                break;
            case ACTION:
                sscanf(value, "%d", action);

                if (LOG_PARSER)
                    log_file(LogMessage, "Parsed action %d", *action); 
                break;
            case TEMPID:
                sscanf(value, "%d", page_num);

                if (LOG_PARSER)
                    log_file(LogMessage, "Parsed temp id %d", *page_num); 
                break;
            case ATTR:
                page_set_page_attrs(engine.hub->pages[*page_num], attr, value);

                if (LOG_PARSER)
                    log_file(LogMessage, "Parsed attr %s with value %s", attr, value); 
                break;
            case VALUE:
                log_file(LogWarn, "Recieved value before attr");
                break;
            default:
                log_file(LogWarn, "Unknown token %d", tok);
        }
    }
}

Token parser_get_token(char *value) {
    char c;
    int i = 0;
    memset(value, '\0', MAX_BUF_SIZE);

    // read chars until we get a '#', '=' or end of message
    while (TRUE) {
        c = parser_get_char(socket_client);

        switch (c) {
            case '=':
                goto EQUAL;
            case '#':
                goto HASH;
            case END_OF_MESSAGE:
                return EOM;
        }

        if (i >= MAX_BUF_SIZE) {
            log_file(LogError, "Parser token buffer out of memory");
        }

        value[i++] = c;
    }

EQUAL:
    // We found an attribute 

    // First check if it part of the message header
    if (strcmp(value, "ver") == 0) {
        return VERSION;
    } else if (strcmp(value, "len") == 0) {
        return LENGTH;
    } else if (strcmp(value, "action") == 0) {
        return ACTION;
    } else if (strcmp(value, "temp") == 0) {
        return TEMPID;
    }

    // Not a header attribute so it is part of the page,
    // we return the attr for the page to deal with
    return ATTR;

HASH:
    // We found a value
    return VALUE;

}

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

