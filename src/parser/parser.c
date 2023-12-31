/*
 * parser.c 
 *
 * Recieve graphics request over tcp from Chroma-Viz.
 *
 */

#include "parser_internal.h"
#include "chroma-typedefs.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

static int socket_client = -1;

ServerResponse  parse_server_get_message(int socket_client);

void    parse_page(IPage *page);
void    parse_header(int *page_num, int *action, int *layer);
Token   parse_get_token(char *value);
char    parse_get_char(int socket_client);
void    parse_clean_buffer(void);

/*
 * Check eng->socket for a tcp connection.
 *
 * If we have an open connection, listen for a message, 
 *    - If we recieve a graphics request, parse it and call the callback on_message(),
 *    - If the listen times out, do nothing,
 *    - If the client closes the connection, close on our end and cleanup buffer.
 *
 */
void parser_read_socket(Engine *eng, int *page_num, int *action, int *layer) {
    if (socket_client < 0) {
        socket_client = parse_client_listen(eng->socket);
    } else {
        ServerResponse rec = parse_server_get_message(socket_client);

        switch (rec) {
        case SERVER_MESSAGE:
            parse_header(page_num, action, layer);
            IPage *page = graphics_hub_get_page(eng->hub, *page_num);

            if (page == NULL) {
                // invalid page, reset globals and clear remaining message
                *page_num = 0;
                *action = BLANK;
                *layer = 0;

                char attr[PARSE_BUF_SIZE];
                while (parse_get_token(attr) != EOM);
                return;
            }

            // Read new page values
            parse_page(page);

            // reset animation times
            graphics_hub_set_time(eng->hub, 0.0f, *layer);
            graphics_hub_set_time(eng->hub, 0.0f, 0);

            // rebuild relative positions
            graphics_page_update_geometry(page);
            break;
        case SERVER_TIMEOUT:
            break;
        case SERVER_CLOSE:
            shutdown(socket_client, SHUT_RDWR);
            parse_clean_buffer();
            socket_client = -1;

            break;
        }
    }
}

/*
 * Parse the header of a gui request 
 */
void parse_header(int *page_num, int *action, int *layer) {
    int parsed_version = 0; 
    int parsed_length = 0;
    int parsed_action = 0;
    int parsed_page_num = 0;

    Token tok;
    int v_m, v_n;
    char attr[PARSE_BUF_SIZE];
    char value[PARSE_BUF_SIZE];

    while ((tok = parse_get_token(attr)) != EOM
           && parse_get_token(value) != EOM) {
        switch (tok) {
            case VERSION:
                sscanf(value, "%d,%d", &v_m, &v_n);
                parsed_version = 1;
                
                if (v_m != 1 || v_n != 4) {
                    log_file(LogError, "Parser", "Incorrect encoding version v%d.%d, expected v1.4", v_m, v_n);
                }

                if (LOG_PARSER)
                    log_file(LogMessage, "Parser", "Message v%d.%d", v_m, v_n); 
                break;
            case LAYER:
                sscanf(value, "%d", layer);
                parsed_length = 1;

                if (LOG_PARSER)
                    log_file(LogMessage, "Parser", "Layer %d", *layer); 
                break;
            case ACTION:
                sscanf(value, "%d", action);
                parsed_action = 1;

                if (LOG_PARSER)
                    log_file(LogMessage, "Parser", "Action %d", *action); 
                break;
            case TEMPID:
                sscanf(value, "%d", page_num);
                parsed_page_num = 1;

                if (LOG_PARSER)
                    log_file(LogMessage, "Parser", "Template id %d", *page_num); 
                break;
            default:
                log_file(LogWarn, "Parser", "Missing header tokens");
        }

        if (parsed_version && parsed_length && parsed_action && parsed_page_num) {
            return;
        }
    }
}

void parse_page(IPage *page) {
    char attr[PARSE_BUF_SIZE], value[PARSE_BUF_SIZE];
    Token tok;
    IGeometry *geo;
    int geo_num = -1;

    while (1) {
        tok = parse_get_token(attr);
        if (tok == EOM) {
            return;
        } else if (tok != ATTR) {
            log_file(LogWarn, "Parser", "Unexpected token %d, expected %d", tok, ATTR);
        }
        
        tok = parse_get_token(value);
        if (tok == EOM) {
            log_file(LogWarn, "Parser", "Parsed attr without a value");
        } else if (tok != VALUE) {
            log_file(LogWarn, "Parser", "Unexpected token %d, expected %d", tok, VALUE);
        }

        if (strncmp(attr, "geo_num", 7) == 0) {
            geo_num = atoi(value);
            continue;
        }

        if (geo_num == -1) {
            log_file(LogError, "Parser", "Didn't find a geo num");
        }

        geo = graphics_page_get_geometry(page, geo_num);
        geometry_set_attr(geo, attr, value);

        if (LOG_PARSER) {
            log_file(LogMessage, "Parser", "\tGeo %d; %s = %s", geo_num, attr, value);
        }
    }
}

Token parse_get_token(char *value) {
    char c;
    int i = 0;
    memset(value, '\0', PARSE_BUF_SIZE);

    // read chars until we get a '#', '=' or end of message
    while (1) {
        c = parse_get_char(socket_client);

        switch (c) {
            case '=':
                goto EQUAL;
            case '#':
                goto HASH;
            case END_OF_MESSAGE:
                return EOM;
        }

        if (i >= PARSE_BUF_SIZE) {
            log_file(LogError, "Parser", "token buffer out of memory");
        }

        value[i++] = c;
    }

EQUAL:
    // We found an attribute 

    // First check if it part of the message header
    if (strcmp(value, "ver") == 0) {
        return VERSION;
    } else if (strcmp(value, "layer") == 0) {
        return LAYER;
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

static char buf[PARSE_BUF_SIZE];
static int buf_ptr = 0;

char parse_get_char(int socket_client) {
    if (parse_server_get_message(socket_client) != SERVER_MESSAGE) {
        log_file(LogWarn, "Parser", "Tried to get char, but didn't have any remaining");
    }

    return buf[buf_ptr++];
}

void parse_clean_buffer(void) {
    buf_ptr = 0;
    memset(buf, '\0', sizeof buf);
}

ServerResponse parse_server_get_message(int socket_client) {
    if (buf_ptr < PARSE_BUF_SIZE && buf[buf_ptr] != '\0') {
        return SERVER_MESSAGE;
    }

    parse_clean_buffer();
    return parse_tcp_recieve_message(socket_client, buf);
}

