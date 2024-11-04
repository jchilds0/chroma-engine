/*
 * parser_recieve_graphics.c 
 *
 * Recieve graphics request over tcp from Chroma-Viz.
 *
 */

#include "chroma-engine.h"
#include "geometry.h"
#include "graphics.h"
#include "graphics/graphics_internal.h"
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

int     parser_parse_page(IPage *page);
int     parser_parse_header(PageStatus *status);
int     parser_get_token(char *value, Token *t);

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
int parser_parse_graphic(Engine *eng, PageStatus *status) {
    ServerResponse rec;
    status->action = BLANK;

    // check if current connection has a message
    if (socket_client >= 0) {
        rec = parser_get_message(socket_client, &buf_ptr, buf);

        switch (rec) {
            case SERVER_MESSAGE:
                goto PAGE;
                break;

            case SERVER_CLOSE:
                shutdown(socket_client, SHUT_RDWR);
                parser_clean_buffer(&buf_ptr, buf);

            case SERVER_TIMEOUT:
                socket_client = -1;
                break;
        }
    }

    // look for messages in other clients
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        rec = parser_get_message(clients[i], &buf_ptr, buf);

        switch (rec) {
            case SERVER_MESSAGE:
                socket_client = clients[i];
                goto PAGE;
                break;

            case SERVER_CLOSE:
                if (socket_client == clients[i]) {
                    socket_client = -1;
                }

                parser_clean_buffer(&buf_ptr, buf);
                shutdown(clients[i], SHUT_RDWR);
                clients[i] = -1;
                break;

            case SERVER_TIMEOUT:
                break;
        }
    }

    return 0;

PAGE:
    log_file(LogMessage, "Parser", "Recieved message from %d", socket_client);

    int start = clock();
    if (parser_parse_header(status) < 0) {
        return -1;
    }

    if (status->action == UPDATE) {
        log_file(LogMessage, "Parser", "Updating template %d", status->temp_id);
        if (parser_update_template(eng, status->temp_id) < 0) {
            return -1;
        }

        status->action = BLANK;
        char attr[PARSE_BUF_SIZE];
        Token t = NONE;
        while (t != EOM) {
            if (parser_get_token(attr, &t) < 0) {
                log_file(LogWarn, "Parser", "Missing EOM tag");
                break;
            }
        };

        int end = clock();
        log_file(LogMessage, "Graphics", "Update template in %f ms", ((double) (end - start) * 1000) / CLOCKS_PER_SEC);

        return 0;
    }

    int page_index = graphics_hub_get_page(&eng->hub, status->temp_id);

    if (page_index < 0) {
        // invalid page, reset globals and clear remaining message
        status->temp_id = -1;
        status->action = BLANK;
        status->layer = 0;

        char attr[PARSE_BUF_SIZE];
        Token t = NONE;
        while (t != EOM) {
            if (parser_get_token(attr, &t) < 0) {
                log_file(LogWarn, "Parser", "Missing EOM tag");
                break;
            }
        };

        return -1;
    }

    IPage *page = &eng->hub.items[page_index];
    // Read new page values
    if (parser_parse_page(page) < 0) {
        return -1;
    }

    switch (status->action) {
        case ANIMATE_ON:
            status->frame_num = 1;
            break;

        case CONTINUE:
            status->frame_num = MIN(status->frame_num + 1, page->max_keyframe - 1);
            break;

        case ANIMATE_OFF:
            status->frame_num = page->max_keyframe - 1;
            break;

        default:
            break;
    }

    for (int i = 0; i < page->len_geometry; i++) {
        IGeometry *geo = page->geometry[i];
        if (geo == NULL) {
            continue;
        }

        if (geo->geo_type != IMAGE) {
            continue;
        }

        if (parser_recieve_image(eng, (GeometryImage *)geo) != SERVER_MESSAGE) {
            log_file(LogWarn, "Parser", "Error receiving image from chroma hub");
        }
    }

    int end = clock();
    log_file(LogMessage, "Graphics", "Parsed Page in %f ms", ((double) (end - start) * 1000) / CLOCKS_PER_SEC);

    start = clock();
    graphics_page_calculate_keyframes(page);
    end = clock();

    log_file(LogMessage, "Graphics", "Calculated keyframes in %f ms", ((double) (end - start) * 1000) / CLOCKS_PER_SEC);
    return 0;
}

/*
 * Parse the header of a gui request 
 */
int parser_parse_header(PageStatus *status) {
    int parsed_version = 0; 
    int parsed_length = 0;
    int parsed_action = 0;
    int parsed_temp_id = 0;

    Token tok_attr, tok_value;
    int v_m, v_n;
    char attr[PARSE_BUF_SIZE];
    char value[PARSE_BUF_SIZE];

    while (1) {
        if (parser_get_token(attr, &tok_attr) < 0) {
            log_file(LogError, "Parser", "Get token %s %d", __FILE__, __LINE__);
            return -1;
        }

        if (parser_get_token(value, &tok_value) < 0) {
            log_file(LogError, "Parser", "Get token %s %d", __FILE__, __LINE__);
            return -1;
        }

        switch (tok_attr) {
            case VERSION:
                sscanf(value, "%d,%d", &v_m, &v_n);
                parsed_version = 1;
                
                if (v_m != 1 || v_n != 4) {
                    log_file(LogError, "Parser", "Incorrect encoding version v%d.%d, expected v1.4", v_m, v_n);
                    return -1;
                }

                if (LOG_PARSER) {
                    log_file(LogMessage, "Parser", "Message v%d.%d", v_m, v_n); 
                }
                break;

            case LAYER:
                sscanf(value, "%d", &status->layer);
                parsed_length = 1;

                if (LOG_PARSER) {
                    log_file(LogMessage, "Parser", "Layer %d", status->layer); 
                }
                break;

            case ACTION:
                sscanf(value, "%d", (int *)&status->action);
                parsed_action = 1;

                if (LOG_PARSER) {
                    log_file(LogMessage, "Parser", "Action %d", status->action); 
                }
                break;

            case TEMPID:
                sscanf(value, "%d", &status->temp_id);
                parsed_temp_id = 1;

                if (LOG_PARSER) {
                    log_file(LogMessage, "Parser", "Template id %d", status->temp_id); 
                }
                break;

            default:
                log_file(LogWarn, "Parser", "Missing header tokens");
        }

        if (parsed_version && parsed_length && parsed_action && parsed_temp_id) {
            return 0;
        }
    }

    log_file(LogError, "Parser", "Didn't find header tokens");
    return -1;
}

int parser_parse_page(IPage *page) {
    char attr[PARSE_BUF_SIZE], value[PARSE_BUF_SIZE];
    Token tok;
    IGeometry *geo;
    int geo_num = -1;

    while (1) {
        if (parser_get_token(attr, &tok) < 0) {
            log_file(LogError, "Parser", "Get token %s %d", __FILE__, __LINE__);
            return -1;
        }

        if (tok == EOM) {
            return 0;
        } else if (tok != ATTR) {
            log_file(LogWarn, "Parser", "Unexpected token %d, expected %d", tok, ATTR);
        }
        
        if (parser_get_token(value, &tok) < 0) {
            log_file(LogError, "Parser", "Get token %s %d", __FILE__, __LINE__);
            return -1;
        }

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
            return -1;
        }

        GeometryAttr geo_attr = geometry_char_to_attr(attr);
        geo = page->geometry[geo_num];

        if (geo_attr < GEO_NUMBER) {
            graphics_graph_update_leaf(&page->keyframe_graph, geo_num, geo_attr, atof(value));
        } else {
            geometry_set_attr(geo, attr, value);
        }

        if (LOG_PARSER) {
            log_file(LogMessage, "Parser", "\tgeo %d: %s = %s", geo_num, attr, value);
        }
    }
}

int parser_get_token(char *value, Token *t) {
    char c;
    int i = 0;
    memset(value, '\0', PARSE_BUF_SIZE);

    // read chars until we get a '#', '=' or end of message
    while (1) {
        if (parser_get_char(socket_client, &buf_ptr, buf, &c) < 0) {
            return -1;
        }

        switch (c) {
            case '=':
                goto EQUAL;
            case '#':
                goto HASH;
            case END_OF_MESSAGE:
                *t = EOM;
                return 0;
        }

        if (i >= PARSE_BUF_SIZE) {
            log_file(LogError, "Parser", "token buffer out of memory");
            return -1;
        }

        value[i++] = c;
    }

EQUAL:
    // We found an attribute 
    *t = ATTR;

    // check if it part of the message header
    if (strcmp(value, "version") == 0) {
        *t = VERSION;
    } else if (strcmp(value, "layer") == 0) {
        *t = LAYER;
    } else if (strcmp(value, "action") == 0) {
        *t = ACTION;
    } else if (strcmp(value, "temp") == 0) {
        *t = TEMPID;
    }

    return 0;

HASH:
    // We found a value
    *t = VALUE;
    return 0;

}

