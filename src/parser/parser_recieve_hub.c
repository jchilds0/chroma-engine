/*
 * parser_recieve_hub.c
 */

#include "chroma-typedefs.h"
#include "geometry.h"
#include "graphics.h"
#include "log.h"
#include "parser_internal.h"
#include <stdlib.h>
#include <string.h>

#define MAX_BUF_SIZE      1024

static char buf[PARSE_BUF_SIZE];
static int buf_ptr = 0;

static int c_token;
static char c_value[PARSE_BUF_SIZE];

void parser_parse_template(IGraphics *hub, int socket_client);
void parser_parse_geometry(IPage *page, int socket_client);
void parser_parse_attribute(IGeometry *geo, int socket_client);

void parser_match_token(int t, int socket_client);
void parser_next_token(int socket_client);

// S -> {'num_temp': num, 'templates': [T]}
void parser_parse_hub(Engine *eng) {
    parser_next_token(eng->hub_socket);
    parser_match_token('{', eng->hub_socket);

    while (c_token == STRING) {
        if (strcmp(c_value, "num_temp") == 0) {
            // 'num_temp': 1234...
            parser_match_token(STRING, eng->hub_socket);
            parser_match_token(':', eng->hub_socket);
            int n = atoi(c_value);

            parser_match_token(INT, eng->hub_socket);
            eng->hub = graphics_new_graphics_hub(n);
            
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "Num Templates: %d", n);
            }

            parser_match_token(',', eng->hub_socket);
        } else if (strcmp(c_value, "templates") == 0) {
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

// T -> {'id': num, 'num_geo': num, 'geometry': [G]} | T, T
void parser_parse_template(IGraphics *hub, int socket_client) {
    IPage *page = NULL;
    int num_geo = -1, 
        temp_id = -1;

    parser_match_token('{', socket_client);

    while (c_token == STRING) {
        if (strcmp(c_value, "id") == 0) {
            // 'num_geo': 1234..
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);

            temp_id = atoi(c_value);
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "\ttemplate id: %d", temp_id);
            }

            parser_match_token(INT, socket_client);
        } else if (strcmp(c_value, "name") == 0) {
            // skip attr
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "\tname: %s", c_value);
            }

            parser_match_token(STRING, socket_client);
        } else if (strcmp(c_value, "layer") == 0) {
            // skip attr
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "\tlayer: %s", c_value);
            }

            parser_match_token(INT, socket_client);
        } else if (strcmp(c_value, "num_geo") == 0) {
            // 'id': 1234..
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);

            num_geo = atoi(c_value);
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "\tnum geometery: %d", num_geo);
            }

            parser_match_token(INT, socket_client);
        } else if (strcmp(c_value, "anim_on") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);

            graphics_page_set_animation(page, "anim_on", c_value);
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "\tanimate on: %s", c_value);
            }

            parser_match_token(STRING, socket_client);
        } else if (strcmp(c_value, "anim_cont") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);

            graphics_page_set_animation(page, "anim_cont", c_value);
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "\tanimate continue: %s", c_value);
            }

            parser_match_token(STRING, socket_client);
        } else if (strcmp(c_value, "anim_off") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);

            graphics_page_set_animation(page, "anim_off", c_value);
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "\tanimate off: %s", c_value);
            }

            parser_match_token(STRING, socket_client);
        } else if (strcmp(c_value, "geometry") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            
            if (page == NULL) {
                log_file(LogError, "Parser", "Template ID or num of geom not specific");
            }

            // 'geometry': [...]
            parser_match_token('[', socket_client);

            parser_parse_geometry(page, socket_client);

            parser_match_token(']', socket_client);
        } else {
            log_file(LogWarn, "Parser", "Unknown template attribute %s", c_value);
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            parser_next_token(socket_client);
        }

        if (temp_id != -1 && num_geo != -1 && page == NULL) {
            page = graphics_hub_add_page(hub, num_geo, temp_id);
        }

        if (c_token == ',') {
            parser_match_token(',', socket_client);
        }
    }

    parser_match_token('}', socket_client);

    if (c_token == ',') {
        parser_match_token(',', socket_client);
        parser_parse_template(hub, socket_client);
    }
}

// G -> {'id': num, 'type': string, 'attr': [A]} | G, G
void parser_parse_geometry(IPage *page, int socket_client) {
    IGeometry *geo;
    int id;
    int got_id = 0,
        got_type = 0;
    char geo_type[PARSE_BUF_SIZE];

    parser_match_token('{', socket_client);

    while (c_token == STRING) {
        if (strcmp(c_value, "id") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            id = atoi(c_value);
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "\tgeometry id: %d", id);
            }

            parser_match_token(INT, socket_client);
            got_id = 1;
        } else if (strcmp(c_value, "name") == 0) {
            // skip attr
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "\t\tname: %s", c_value);
            }

            parser_match_token(STRING, socket_client);
        } else if (strcmp(c_value, "geo_type") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            memcpy(geo_type, c_value, PARSE_BUF_SIZE);
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "\t\tgeo type: %s", geo_type);
            }

            parser_match_token(STRING, socket_client);
            got_type = 1;
        } else if (strcmp(c_value, "prop_type") == 0) {
            // skip attr
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "\t\tprop type: %s", c_value);
            }

            parser_match_token(STRING, socket_client);
        } else if (strcmp(c_value, "visible") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            parser_match_token('[', socket_client);

            while (c_token == STRING) {
                parser_match_token(STRING, socket_client);
                parser_match_token(':', socket_client);
                parser_match_token(STRING, socket_client);

                if (c_token == ',') {
                    parser_match_token(',', socket_client);
                }
            }

            parser_match_token(']', socket_client);
        } else if (strcmp(c_value, "attr") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);

            if (!(got_type && got_id)) {
                log_file(LogWarn, "Parser", "Missing geometry attributes");
                return;
            }

            parser_match_token('[', socket_client);

            geo = graphics_page_add_geometry(page, id, geo_type);
            parser_parse_attribute(geo, socket_client);

            if (strcmp(geo_type, "rect") == 0 && geometry_get_int_attr(geo, "parent") == 0) {
                graphics_page_set_bg_index(page, id);

                if (LOG_PARSER) {
                    log_file(LogMessage, "Parser", "BG Index = %d", id);
                }
            }

            parser_match_token(']', socket_client);
        } else {
            log_file(LogWarn, "Parser", "Unknown geometry attribute %s", c_value);
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            parser_next_token(socket_client);
        }

        if (c_token == ',') {
            parser_match_token(',', socket_client);
        }
    }

    parser_match_token('}', socket_client);

    if (c_token == ',') {
        parser_match_token(',', socket_client);
        parser_parse_geometry(page, socket_client);
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
        if (strcmp(c_value, "name") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            memcpy(name, c_value, PARSE_BUF_SIZE);
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "\t\t\tname: %s", name);
            }

            parser_match_token(STRING, socket_client);

            got_name = 1;
        } else if (strcmp(c_value, "value") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            memcpy(value, c_value, PARSE_BUF_SIZE);
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "\t\t\tvalue: %s", value);
            }
            
            parser_match_token(STRING, socket_client);

            got_value = 1;
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

    if (c_token == ',') {
        parser_match_token(',', socket_client);
        parser_parse_attribute(geo, socket_client);
    }
}

void parser_match_token(int t, int socket_client) {
    if (t == c_token) {
        parser_next_token(socket_client);
    } else {
        log_file(LogError, "Parser", "Couldn't match token %d to token %d", c_token, t);
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
    } else if (c >= '0' && c <= '9') {
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

