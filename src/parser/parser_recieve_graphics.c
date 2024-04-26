/*
 * parser_recieve_graphics.c 
 *
 * Recieve graphics request over tcp from Chroma-Viz.
 *
 */

#include "geometry.h"
#include "graphics.h"
#include "parser_internal.h"
#include "chroma-typedefs.h"
#include "log.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

static int clients[MAX_CONNECTIONS];
static int socket_client = -1;

void    parser_page(IPage *page);
void    parser_header(int *temp_id, int *action, int *layer);
Token   parser_get_token(char *value);

static char buf[PARSE_BUF_SIZE];
static int buf_ptr = 0;

void parser_init_sockets(void) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        clients[i] = -1;
    }
}

void parser_check_socket(int server_socket) {
    int client_sock;
    socklen_t client_size;
    struct sockaddr_in client_addr;

    client_size = sizeof client_addr;
    client_sock = accept(server_socket, (struct sockaddr *) &client_addr, &client_size);

    if (client_sock < 0) {
        // no new clients
        return;
    }

    int i;
    for (i = 0; i < MAX_CONNECTIONS; i++) {
        if (clients[i] >= 0) {
            continue;
        }

        log_file(LogMessage, "Parser", "New client %d:%d", client_addr.sin_addr, client_addr.sin_port);
        clients[i] = client_sock;
        break;
    }

    if (i == MAX_CONNECTIONS) {
        log_file(LogWarn, "Parser", "Max clients connected");
    }
}

/*
 * Check clients array for messages, first checking the current client;
 *
 * If we have an open connection, listen for a message, 
 *    - If we recieve a graphics request, parse the page,
 *    - If the listen times out, move on to the next client,
 *    - If the client closes the connection, close on our end.
 *
 */

void parser_parse_graphic(Engine *eng, int *temp_id, int *action, int *layer) {
    ServerResponse rec;

    // check if current connection has a message
    if (socket_client >= 0) {
        rec = parser_get_message(socket_client, &buf_ptr, buf);

        if (rec == SERVER_MESSAGE) {
            goto PAGE;
        }
    }

    // look for messages in other clients
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        rec = parser_get_message(clients[i], &buf_ptr, buf);

        switch (rec) {
            case SERVER_MESSAGE:
                socket_client = clients[i];
                goto PAGE;
            case SERVER_CLOSE:
                if (socket_client == clients[i]) {
                    socket_client = -1;
                }

                shutdown(clients[i], SHUT_RDWR);
                clients[i] = -1;
            case SERVER_TIMEOUT:
                break;
        }
    }

    return;

PAGE:
    log_file(LogMessage, "Parser", "Recieved message from %d", socket_client);

    int start = clock();
    parser_header(temp_id, action, layer);
    IPage *page = graphics_hub_get_page(eng->hub, *temp_id);

    if (page == NULL) {
        // invalid page, reset globals and clear remaining message
        *temp_id = -1;
        *action = BLANK;
        *layer = 0;

        char attr[PARSE_BUF_SIZE];
        while (parser_get_token(attr) != EOM);
        return;
    }

    // Read new page values
    parser_page(page);
    graphics_hub_set_time(eng->hub, 0.0f, *layer);
    graphics_page_calculate_keyframes(page);

    if (*action == UPDATE) {
        parser_update_template(eng, *temp_id);
        *action = BLANK;
        return;
    }

    int num_geo = graphics_page_num_geometry(page);
    for (int i = 0; i < num_geo; i++) {
        IGeometry *geo = graphics_page_get_geometry(page, i);
        if (geo == NULL) {
            continue;
        }

        if (geo->geo_type != IMAGE) {
            continue;
        }

        if (parser_recieve_image(eng->hub_socket, (GeometryImage *)geo) != SERVER_MESSAGE) {
            log_file(LogWarn, "Parser", "Error receiving image from chroma hub");
        }
    }

    int end = clock();
    log_file(LogMessage, "Graphics", "Parsed Page in %f ms", ((double) (end - start) * 1000) / CLOCKS_PER_SEC);
}

/*
 * Parse the header of a gui request 
 */
void parser_header(int *temp_id, int *action, int *layer) {
    int parsed_version = 0; 
    int parsed_length = 0;
    int parsed_action = 0;
    int parsed_temp_id = 0;

    Token tok;
    int v_m, v_n;
    char attr[PARSE_BUF_SIZE];
    char value[PARSE_BUF_SIZE];

    while ((tok = parser_get_token(attr)) != EOM
           && parser_get_token(value) != EOM) {
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
                sscanf(value, "%d", temp_id);
                parsed_temp_id = 1;

                if (LOG_PARSER)
                    log_file(LogMessage, "Parser", "Template id %d", *temp_id); 
                break;
            default:
                log_file(LogWarn, "Parser", "Missing header tokens");
        }

        if (parsed_version && parsed_length && parsed_action && parsed_temp_id) {
            return;
        }
    }
}

void parser_page(IPage *page) {
    char attr[PARSE_BUF_SIZE], value[PARSE_BUF_SIZE];
    Token tok;
    IGeometry *geo;
    int geo_num = -1;

    while (1) {
        tok = parser_get_token(attr);
        if (tok == EOM) {
            return;
        } else if (tok != ATTR) {
            log_file(LogWarn, "Parser", "Unexpected token %d, expected %d", tok, ATTR);
        }
        
        tok = parser_get_token(value);
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
            log_file(LogMessage, "Parser", "\tgeo %d; %s = %s", geo_num, attr, value);
        }
    }
}

Token parser_get_token(char *value) {
    char c;
    int i = 0;
    memset(value, '\0', PARSE_BUF_SIZE);

    // read chars until we get a '#', '=' or end of message
    while (1) {
        c = parser_get_char(socket_client, &buf_ptr, buf);

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

