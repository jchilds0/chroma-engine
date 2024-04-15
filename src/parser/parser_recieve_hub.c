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
void parser_parse_keyframe(IPage *page, int socket_client);
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
        num_keyframe = -1,
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
            // 'num_geo': 1234..
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);

            num_geo = atoi(c_value);
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "\tnum geometery: %d", num_geo);
            }

            parser_match_token(INT, socket_client);
        } else if (strcmp(c_value, "num_keyframe") == 0) {
            // 'num_keyframe': 1234
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);

            num_keyframe = atoi(c_value);
            if (LOG_PARSER) {
                log_file(LogMessage, "Parser", "\tnum keyframe: %d", num_keyframe);
            }

            parser_match_token(INT, socket_client);
        } else if (strcmp(c_value, "keyframe") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);

            if (page == NULL) {
                log_file(LogError, "Parser", "Template ID, num of geom or keyframes not specific");
            }

            // 'keyframe': [...]
            parser_match_token('[', socket_client);

            parser_parse_keyframe(page, socket_client);
            graphics_page_keyframe(page);

            parser_match_token(']', socket_client);
        } else if (strcmp(c_value, "geometry") == 0) {
            parser_match_token(STRING, socket_client);
            parser_match_token(':', socket_client);
            
            if (page == NULL) {
                log_file(LogError, "Parser", "Template ID, num of geom or keyframes not specific");
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

        if (temp_id != -1 && num_geo != -1 && num_keyframe != -1 && page == NULL) {
            page = graphics_hub_add_page(hub, num_geo, num_keyframe, temp_id);
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

// K -> {'frame_num': num, ...
void parser_parse_keyframe(IPage *page, int socket_client) {
    char buf[GEO_BUF_SIZE];
    int frame_num, frame_geo, bind_frame, bind_geo, keyframe;


    while (c_token == '{') {
        parser_match_token('{', socket_client);

        frame_num  = -1;
        frame_geo  = -1;
        bind_frame = -1;
        bind_geo   = -1;
        keyframe   = 0;

        while (c_token == STRING) {
            if (strcmp(c_value, "frame_num") == 0) {
                parser_match_token(STRING, socket_client);
                parser_match_token(':', socket_client);

                frame_num = atoi(c_value);
                if (LOG_PARSER) {
                    log_file(LogMessage, "Parser", "\t\tframe num: %d", frame_num);
                }

                parser_match_token(INT, socket_client);
            } else if (strcmp(c_value, "frame_geo") == 0) {
                parser_match_token(STRING, socket_client);
                parser_match_token(':', socket_client);

                frame_geo = atoi(c_value);
                if (LOG_PARSER) {
                    log_file(LogMessage, "Parser", "\t\tframe geo: %d", frame_geo);
                }

                parser_match_token(INT, socket_client);
            } else if (strcmp(c_value, "frame_attr") == 0) {
                parser_match_token(STRING, socket_client);
                parser_match_token(':', socket_client);

                memcpy(buf, c_value, sizeof buf);
                if (LOG_PARSER) {
                    log_file(LogMessage, "Parser", "\t\tframe attr: %s", buf);
                }

                parser_match_token(STRING, socket_client);
            } else if (strcmp(c_value, "value") == 0) {
                parser_match_token(STRING, socket_client);
                parser_match_token(':', socket_client);

                if (keyframe) {
                    log_file(LogWarn, "Parser", "Keyframe has multiple types");
                }

                if (LOG_PARSER) {
                    log_file(LogMessage, "Parser", "\t\tkey frame set value: %s", c_value);
                }

                int value = atoi(c_value);
                graphics_page_add_keyframe_value(page, frame_num, frame_geo, buf, value);
                keyframe = 1;

                parser_match_token(INT, socket_client);
            } else if (strcmp(c_value, "user_frame") == 0) {
                parser_match_token(STRING, socket_client);
                parser_match_token(':', socket_client);

                if (keyframe) {
                    log_file(LogWarn, "Parser", "Keyframe has multiple types");
                }

                if (LOG_PARSER) {
                    log_file(LogMessage, "Parser", "\t\tkey frame user value: %s", c_value);
                }

                graphics_page_add_keyframe_user(page, frame_num, frame_geo, buf);
                keyframe = 1;

                parser_match_token(STRING, socket_client);
            } else if (strcmp(c_value, "bind_frame") == 0) {
                parser_match_token(STRING, socket_client);
                parser_match_token(':', socket_client);

                bind_frame = atoi(c_value);
                if (LOG_PARSER) {
                    log_file(LogMessage, "Parser", "\t\tbind frame: %d", bind_frame);
                }

                parser_match_token(INT, socket_client);
            } else if (strcmp(c_value, "bind_geo") == 0) {
                parser_match_token(STRING, socket_client);
                parser_match_token(':', socket_client);

                bind_geo = atoi(c_value);
                if (LOG_PARSER) {
                    log_file(LogMessage, "Parser", "\t\tbind geo: %d", bind_geo);
                }

                parser_match_token(INT, socket_client);
            } else if (strcmp(c_value, "bind_attr") == 0) {
                parser_match_token(STRING, socket_client);
                parser_match_token(':', socket_client);

                if (keyframe) {
                    log_file(LogWarn, "Parser", "Keyframe has multiple types");
                } else if (bind_frame == -1 || bind_geo == -1) {
                    log_file(LogWarn, "Parser", "Keyframe bind missing attrs");
                }

                keyframe = 1;

                if (LOG_PARSER) {
                    log_file(LogMessage, "Parser", "\t\tkeyframe bind to frame %d geo %d attr %s", bind_frame, bind_geo, c_value);
                }

                graphics_page_add_keyframe_bind(page, frame_num, frame_geo, buf, bind_frame, bind_geo, c_value);
            } else {
                log_file(LogWarn, "Parser", "Unknown keyframe attribute %s", c_value);
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
        }
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

