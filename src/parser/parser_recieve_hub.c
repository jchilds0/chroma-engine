/*
 * parser_recieve_hub.c
 */

#include "chroma-typedefs.h"
#include "geometry.h"
#include "graphics.h"
#include "graphics/graphics_internal.h"
#include "log.h"
#include "parser_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define MAX_BUF_SIZE      1024

static char buf[PARSE_BUF_SIZE];
static int buf_ptr = 0;

static int c_token;
static char c_value[PARSE_BUF_SIZE];

void parser_parse_keyframe(IPage *page, int socket_client);
void parser_parse_template(IGraphics *hub, int socket_client);
void parser_parse_geometry(IPage *page, int socket_client, int geo_type);
void parser_parse_attribute(IGeometry *geo, int socket_client);

void parser_match_token(int t, int socket_client);
void parser_next_token(int socket_client);

// S -> {'num_temp': num, 'templates': [T]}
void parser_parse_hub(Engine *eng) {
    parser_next_token(eng->hub_socket);
    parser_match_token('{', eng->hub_socket);

    while (c_token == STRING) {
        if (strcmp(c_value, "NumTemplates") == 0) {
            // 'num_temp': 1234...
            parser_match_token(STRING, eng->hub_socket);
            parser_match_token(':', eng->hub_socket);
            int n = atoi(c_value);

            parser_match_token(INT, eng->hub_socket);
            eng->hub = graphics_new_graphics_hub(n);
            
            if (LOG_TEMPLATE) {
                log_file(LogMessage, "Parser", "Num Templates: %d", n);
            }

            parser_match_token(',', eng->hub_socket);
        } else if (strcmp(c_value, "Templates") == 0) {
            // 'templates': [...]
            parser_match_token(STRING, eng->hub_socket);
            parser_match_token(':', eng->hub_socket);
            parser_match_token('[', eng->hub_socket);
            if (c_token == ']') {
                parser_match_token(']', eng->hub_socket);
                break;
            }

            parser_parse_template(eng->hub, eng->hub_socket);

            parser_match_token(']', eng->hub_socket);
        } else {
            log_file(LogWarn, "Parser", "Unknown template attribute %s", c_value);
            parser_match_token(STRING, eng->hub_socket);
            parser_match_token(':', eng->hub_socket);
            parser_next_token(eng->hub_socket);
        }

        if (c_token == ',') {
            parser_match_token(',', eng->hub_socket);
        }
    }

    if (c_token != '}') {
        log_file(LogError, "Parser", "Couldn't match token %d to token %d", c_token, '}');
    }
}

void parser_update_template(Engine *eng, int temp_id) {
    char msg[MAX_BUF_SIZE];
    memset(msg, '\0', sizeof msg);

    graphics_hub_free_page(eng->hub, temp_id);

    sprintf(msg, "ver 0 1 temp %d;", temp_id);
    if (send(eng->hub_socket, msg, strlen(msg), 0) < 0) {
        log_file(LogError, "Parser", "Error requesting template %d", temp_id); 
    }

    log_file(LogMessage, "Parser", "Requesting template %d", temp_id);

    parser_clean_buffer(&buf_ptr, buf);
    parser_next_token(eng->hub_socket);

    if (c_token == END_OF_MESSAGE) {
        log_file(LogMessage, "Parser", "Error in chroma hub");
        return;
    }

    parser_parse_template(eng->hub, eng->hub_socket);
}

// T -> {'id': num, 'num_geo': num, 'geometry': [G]} | T, T
void parser_parse_template(IGraphics *hub, int socket_client) {
    IPage *page = graphics_new_page();
    int geo_type, key_type;

    parser_match_token('{', socket_client);

    while (c_token == STRING) {
        if (strcmp(c_value, "TempID") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);

            page->temp_id = atoi(c_value);
            if (LOG_TEMPLATE) {
                log_file(LogMessage, "Parser", "\ttemplate id: %d", page->temp_id);
            }

            parser_match_token(INT, socket_client);
        } else if (strcmp(c_value, "Name") == 0) {
            // skip attr
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            if (LOG_TEMPLATE) {
                log_file(LogMessage, "Parser", "\tname: %s", c_value);
            }

            parser_match_token(STRING, socket_client);
        } else if (strcmp(c_value, "Layer") == 0) {
            // skip attr
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            if (LOG_TEMPLATE) {
                log_file(LogMessage, "Parser", "\tlayer: %s", c_value);
            }

            parser_match_token(INT, socket_client);
        } else if (strcmp(c_value, "NumGeometry") == 0) {
            // 'num_geo': 1234..
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);

            if (LOG_TEMPLATE) {
                log_file(LogMessage, "Parser", "\tnum geometery: %d", atoi(c_value));
            }

            parser_match_token(INT, socket_client);
        } else if (strcmp(c_value, "NumKeyframe") == 0) {
            // 'num_keyframe': 1234
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);

            if (LOG_TEMPLATE) {
                log_file(LogMessage, "Parser", "\tnum keyframe: %d", atoi(c_value));
            }

            parser_match_token(INT, socket_client);
        } else if ((key_type = graphics_keyframe_type(c_value)) >= 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);

            if (page == NULL) {
                log_file(LogError, "Parser", "Page not initialised");
            }

            // 'keyframe': [...]
            parser_match_token('[', socket_client);
            parser_parse_keyframe(page, socket_client);
            parser_match_token(']', socket_client);

        } else if ((geo_type = geometry_geo_type(c_value)) >= 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            
            if (page == NULL) {
                log_file(LogError, "Parser", "Page not initialised");
            }

            // 'geometry': [...]
            parser_match_token('[', socket_client);
            parser_parse_geometry(page, socket_client, geo_type);
            parser_match_token(']', socket_client);
        } else {
            log_file(LogWarn, "Parser", "Unknown template attribute %s", c_value);
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            parser_next_token(socket_client);
        }

        if (c_token == ',') {
            parser_match_token(',', socket_client);
        }
    }

    parser_match_token('}', socket_client);
    graphics_page_generate(page);
    graphics_page_default_relations(page);

    if (c_token == ',') {
        parser_match_token(',', socket_client);
        parser_parse_template(hub, socket_client);
    }
}

// K -> {'frame_num': num, ...
void parser_parse_keyframe(IPage *page, int socket_client) {
    if (c_token == ']') {
        return;
    }

    static int frameIndex = 0;
    Keyframe *frame = graphics_page_add_keyframe(page);
    char name[GEO_BUF_SIZE];
    int value;

    while (c_token == '{') {
        parser_match_token('{', socket_client);

        frame->expand = 0;

        while (c_token == STRING) {
            memcpy(name, c_value, sizeof name);
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);

            if (LOG_TEMPLATE) {
                log_file(LogMessage, "Parser", "keyframe %d: %s = %s", frameIndex, name, c_value);
            }

            if (c_token == INT) {
                value = atoi(c_value);
                graphics_keyframe_set_int(frame, name, value);
                parser_match_token(INT, socket_client);
            } else if (c_token == STRING) {
                graphics_keyframe_set_attr(frame, name, c_value);
                parser_match_token(STRING, socket_client);
            } else {
                log_file(LogWarn, "Parser", "Unknown value type %d", c_token);
                parser_next_token(socket_client);
            }

            if (c_token == ',') {
                parser_match_token(',', socket_client);
            }
        }

        parser_match_token('}', socket_client);

        frameIndex++;

        if (c_token == ',') {
            parser_match_token(',', socket_client);
        }
    }
}

// G -> {'id': num, 'type': string, 'attr': [A]} | G, G
void parser_parse_geometry(IPage *page, int socket_client, int geo_type) {
    if (c_token == ']') {
        return;
    }

    IGeometry *geo = graphics_page_add_geometry(page, geo_type);
    parser_match_token('{', socket_client);

    while (c_token == STRING) {
        if (strcmp(c_value, "GeometryID") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            geo->geo_id = atoi(c_value);
            if (LOG_TEMPLATE) {
                log_file(LogMessage, "Parser", "\tgeometry id: %d", geo->geo_id);
            }

            parser_match_token(INT, socket_client);
        } else if (strcmp(c_value, "Name") == 0) {
            // skip attr
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            if (LOG_TEMPLATE) {
                log_file(LogMessage, "Parser", "\t\tname: %s", c_value);
            }

            parser_match_token(STRING, socket_client);
        } else if (strcmp(c_value, "GeoType") == 0) {
            // skip attr
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            if (LOG_TEMPLATE) {
                log_file(LogMessage, "Parser", "\t\tgeo type: %s", c_value);
            }

            parser_match_token(STRING, socket_client);

        } else {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);

            parser_parse_attribute(geo, socket_client);
        }

        if (c_token == ',') {
            parser_match_token(',', socket_client);
        }
    }

    parser_match_token('}', socket_client);

    if (c_token == ',') {
        parser_match_token(',', socket_client);
        parser_parse_geometry(page, socket_client, geo_type);
    }
}

// A -> {'name': string, 'value': string} | A, A
void parser_parse_attribute(IGeometry *geo, int socket_client) {
    char name[PARSE_BUF_SIZE], value[PARSE_BUF_SIZE];
    int got_name = 0, got_value = 0;

    memset(name, '\0', sizeof name);
    memset(value, '\0', sizeof value);

    parser_match_token('{', socket_client);

    while (c_token == STRING) {
        if (strcmp(c_value, "Name") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            memcpy(name, c_value, PARSE_BUF_SIZE);
            if (LOG_TEMPLATE) {
                log_file(LogMessage, "Parser", "\t\t\tname: %s", name);
            }

            parser_match_token(STRING, socket_client);

            got_name = 1;
        } else if (strcmp(c_value, "Value") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            memcpy(value, c_value, PARSE_BUF_SIZE);
            if (LOG_TEMPLATE) {
                log_file(LogMessage, "Parser", "\t\t\tvalue: %s", value);
            }
            
            parser_next_token(socket_client);

            got_value = 1;
        } else {
            // skip attr
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            parser_next_token(socket_client);
        }

        if (c_token == ',') {
            parser_match_token(',', socket_client);
        }
    }

    parser_match_token('}', socket_client);

    if (!got_name || !got_value) {
        log_file(LogWarn, "Parser", "Missing attributes for geometry attribute");
        return;
    }

    geometry_set_attr(geo, name, value);
}

void parser_match_token(int t, int socket_client) {
    if (t == c_token) {
        parser_next_token(socket_client);
    } else {
        parser_incorrect_token(t, c_token, buf, buf_ptr);
    }
}

void parser_next_token(int socket_client) {
    static char c = -1;
    int i = 0;
    memset(c_value, '\0', PARSE_BUF_SIZE);

    if (c == -1) {
        c = parser_get_char(socket_client, &buf_ptr, buf);
    }

    // skip whitespace
    while (c == ' ' || c == '\t' || c == '\n') {
        c = parser_get_char(socket_client, &buf_ptr, buf);
    }

    if (c == '\'') {
        // attr 
        while ((c = parser_get_char(socket_client, &buf_ptr, buf)) != '\'') {
            if (i >= PARSE_BUF_SIZE) {
                log_file(LogError, "Parser", "Parser ran out of memory");
            }

            c_value[i++] = c;
        }

        c = -1;
        c_token = STRING;
        //log_file(LogMessage, "Parser", "String: %s", c_value);
        return;
    } else if (c == '"') {
        // attr 
        while ((c = parser_get_char(socket_client, &buf_ptr, buf)) != '"') {
            if (i >= PARSE_BUF_SIZE) {
                log_file(LogError, "Parser", "Parser ran out of memory");
            }

            c_value[i++] = c;
        }

        c = -1;
        c_token = STRING;
        //log_file(LogMessage, "Parser", "String: %s", c_value);
        return;
    } else if ((c >= '0' && c <= '9') || c == '-') {
        // number
        while (1) {
            c_value[i++] = c;
            c = parser_get_char(socket_client, &buf_ptr, buf);

            if (c < '0' || c > '9') {
                c_token = INT;
                //log_file(LogMessage, "Parser", "Int: %s", c_value);
                return;
            }
        }
    }

    c_token = c;
    c = -1;
    //log_file(LogMessage, "Parser", "Token: %d", c_token);
}

