/*
 * parser_json.c
 *
 */

#include "parser_json.h"
#include "chroma-engine.h"
#include "log.h"
#include "parser/parser_http.h"
#include "parser_internal.h"
#include <stdio.h>
#include <string.h>

JSONArena json_arena = {0, DA_INIT_CAPACITY, NULL, {0, DA_INIT_CAPACITY, NULL}};

static char buf[PARSE_BUF_SIZE];
static int buf_ptr = 0;

static int c_token;
static char c_value[PARSE_BUF_SIZE];

static size_t parser_json_parse_node(HTTPHeader *header);
static void parser_json_parse_object(HTTPHeader *header, JSONNode *node);
static void parser_json_parse_array(HTTPHeader *header, JSONNode *node);

static void parser_match_token(int t, HTTPHeader *header);
static void parser_match_char(HTTPHeader *header, char c2);
static void parser_json_next_token(HTTPHeader *header);

#define JSON_ARRAY(array_obj, data, f)                                                     \
    do {                                                                                   \
        log_assert((array_obj)->type == JSON_ARRAY, "Parser", "Node is not a JSON_ARRAY"); \
        for (int i = 0; i < (array_obj)->array.num_items; i++) {                           \
            size_t node_index = json_arena.objects.items[(array_obj)->array.start + i];    \
            JSONNode *node = &json_arena.items[node_index];                                \
            f((node), (data));                                                             \
        }                                                                                  \
    } while (0);                                                                           \


JSONNode *parser_json_attribute(JSONNode *obj, const char *name) {
    if (obj == NULL || obj->type != JSON_OBJECT) {
        log_file(LogError, "Parser", "JSON node is not an object");
        return NULL;
    }

    for (int i = 0; i < obj->array.num_items; i++) {
        size_t index = json_arena.objects.items[obj->array.start + i];
        JSONNode *attr_node = &json_arena.items[index];

        if (strncmp(attr_node->name, name, MAX_NAME_LENGTH)) {
            continue;
        }

        return attr_node;
    }

    return NULL;
}

int parser_json_get_int(JSONNode *obj, char *name) {
    JSONNode *node = parser_json_attribute(obj, name);
    if (node == NULL || node->type != JSON_INT) {
        log_file(LogWarn, "Parser", "%s is not a JSON Integer", name);
        return 0;
    }

    return node->integer;
}

char *parser_json_get_string(JSONNode *obj, char *name) {
    JSONNode *node = parser_json_attribute(obj, name);
    if (node == NULL || node->type != JSON_STRING) {
        log_file(LogWarn, "Parser", "%s is not a JSON String", name);
        return NULL;
    }

    return node->string;
}

float parser_json_get_float(JSONNode *obj, char *name) {
    JSONNode *node = parser_json_attribute(obj, name);
    if (node == NULL || node->type != JSON_FLOAT) {
        log_file(LogWarn, "Parser", "%s is not a JSON Float", name);
        return 0;
    }

    return node->f;
}

unsigned char parser_json_get_bool(JSONNode *obj, char *name) {
    JSONNode *node = parser_json_attribute(obj, name);
    if (node == NULL) {
        log_file(LogWarn, "Parser", "%s is not a JSON Bool", name);
        return 0;
    }

    if (node->type == JSON_TRUE) {
        return 1;
    } else if (node->type == JSON_FALSE) {
        return 0;
    } else {
        log_file(LogWarn, "Parser", "%s is not a JSON Bool", name);
        return 0;
    }
}

JSONNode *parser_receive_json(int socket_client) {
    if (json_arena.items == NULL) {
        json_arena.items = NEW_ARRAY(DA_INIT_CAPACITY, JSONNode);
        json_arena.objects.items = NEW_ARRAY(DA_INIT_CAPACITY, size_t);
    }

    log_assert(json_arena.count == 0, "Parser", "JSON Arena was not cleaned up");
    parser_clean_buffer(&buf_ptr, buf);

    HTTPHeader *header = parser_http_new_header(socket_client);
    parser_http_header(header, &buf_ptr, buf);

    parser_json_next_token(header);
    size_t root_index = parser_json_parse_node(header);
    parser_http_free_header(header);

    return &json_arena.items[root_index];
}

void parser_clean_json(void) {
    if (json_arena.items == NULL || json_arena.objects.items == NULL) {
        return;
    }

    json_arena.count = 0;
    json_arena.objects.count = 0;
}

static size_t parser_json_parse_node(HTTPHeader *header) {
    JSONNode node;
    memset(node.name, '\0', sizeof node.name);

    switch (c_token) {
        case T_NONE:
            parser_json_next_token(header);
            node.type = JSON_NULL;
            break;

        case T_STRING:
            node.type = JSON_STRING;
            
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "String %s", c_value);
            }

            memset(node.string, '\0', sizeof node.string);
            memcpy(node.string, c_value, sizeof node.string);
            parser_json_next_token(header);
            break;

        case T_INT:
            node.type = JSON_INT;
            node.integer = atoi(c_value);
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "Int %d", node.integer);
            }

            parser_json_next_token(header);
            break;

        case T_FLOAT:
            node.type = JSON_FLOAT;
            node.f = atof(c_value);
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "Float %f", node.f);
            }

            parser_json_next_token(header);
            break;

        case T_FALSE:
            node.type = JSON_FALSE;
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "Bool False");
            }

            parser_json_next_token(header);
            break;

        case T_TRUE:
            node.type = JSON_TRUE;
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "Bool True");
            }

            parser_json_next_token(header);
            break;

        case '{':
            node.type = JSON_OBJECT;
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "JSON Object Start");
            }

            parser_json_parse_object(header, &node);
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "JSON Object End");
            }

            parser_match_token('}', header);
            break;

        case '[':
            node.type = JSON_ARRAY;
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "JSON Array Start");
            }

            parser_json_parse_array(header, &node);
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "JSON Array End");
            }

            parser_match_token(']', header);
            break;

        case T_EOM:
            log_file(LogMessage, "Parser", "End of json");
            break;

        default:
            log_file(LogError, "Parser", "Unknown json token %c", c_token); 
    }

    size_t index = json_arena.count;
    DA_APPEND(&json_arena, node);
    return index;
}

static void parser_json_parse_array(HTTPHeader *header, JSONNode *node) {
    parser_match_token('[', header);
    JSONIndices indices;
    indices.count = 0;
    indices.capacity = 128;
    indices.items = NEW_ARRAY(indices.capacity, size_t);

    while (c_token != ']') {
        size_t item_index = parser_json_parse_node(header);
        DA_APPEND(&indices, item_index);
        if (c_token == ',') {
            parser_match_token(',', header);
        }
    }

    node->array.start = json_arena.objects.count;
    node->array.num_items = indices.count;

    for (int i = 0; i < indices.count; i++) {
        DA_APPEND(&json_arena.objects, indices.items[i]);
    }
}

static void parser_json_parse_object(HTTPHeader *header, JSONNode *node) {
    JSONIndices indices;
    char attr_name[MAX_NAME_LENGTH];
    indices.count = 0;
    indices.capacity = 128;
    indices.items = NEW_ARRAY(indices.capacity, size_t);

    parser_match_token('{', header);

    while (c_token != '}') {
        memset(attr_name, '\0', sizeof attr_name);
        memcpy(attr_name, c_value, sizeof attr_name);

        parser_match_token(T_STRING, header);
        parser_match_token(':', header);

        size_t item_index = parser_json_parse_node(header);
        DA_APPEND(&indices, item_index);

        char *name = json_arena.items[item_index].name;
        memcpy(name, attr_name, sizeof attr_name);

        if (c_token == ',') {
            parser_match_token(',', header);
        }
    }

    node->array.start = json_arena.objects.count;
    node->array.num_items = indices.count;

    for (int i = 0; i < indices.count; i++) {
        DA_APPEND(&json_arena.objects, indices.items[i]);
    }
}

static void parser_match_token(int t, HTTPHeader *header) {
    if (t == c_token) {
        parser_json_next_token(header);
    } else {
        parser_incorrect_token(t, c_token, buf_ptr, buf);
    }
}

static void parser_match_char(HTTPHeader *header, char c2) {
    char c1 = parser_http_get_char(header, &buf_ptr, buf);

    if (c1 == c2) {
        return ;
    } 

    log_file(LogError, "Parser", "Couldn't match char %c to char %c", c1, c2);
}

static void parser_json_next_token(HTTPHeader *header) {
    static char c = -1;
    int i = 0;
    memset(c_value, '\0', PARSE_BUF_SIZE);

    if (c == -1) {
        c = parser_http_get_char(header, &buf_ptr, buf);
    }

    // skip whitespace
    while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        c = parser_http_get_char(header, &buf_ptr, buf);
    }

    if (c == '\'') {
        // attr 
        while ((c = parser_http_get_char(header, &buf_ptr, buf)) != '\'') {
            if (i >= PARSE_BUF_SIZE) {
                log_file(LogError, "Parser", "Parser ran out of memory");
            }

            c_value[i++] = c;
        }

        c = -1;
        c_token = T_STRING;
        //log_file(LogMessage, "Parser", "String: %s", c_value);
        return;
    } else if (c == '"') {
        // attr 
        while ((c = parser_http_get_char(header, &buf_ptr, buf)) != '"') {
            if (i >= PARSE_BUF_SIZE) {
                log_file(LogError, "Parser", "Parser ran out of memory");
            }

            c_value[i++] = c;
        }

        c = -1;
        c_token = T_STRING;
        //log_file(LogMessage, "Parser", "String: %s", c_value);
        return;
    } else if ((c >= '0' && c <= '9') || c == '-') {
        // number
        c_token = T_INT;

        while (1) {
            c_value[i++] = c;
            c = parser_http_get_char(header, &buf_ptr, buf);

            if (c == '.') {
                c_token = T_FLOAT;
            } else if (c < '0' || c > '9') {
                //log_file(LogMessage, "Parser", "Number: %s", c_value);
                return;
            }
        }
    } else if (c == 't' || c == 'T') {
        parser_match_char(header, 'r');
        parser_match_char(header, 'u');
        parser_match_char(header, 'e');
        c = -1;
        c_token = T_TRUE;

        return;
    } else if (c == 'f' || c == 'F') {
        parser_match_char(header, 'a');
        parser_match_char(header, 'l');
        parser_match_char(header, 's');
        parser_match_char(header, 'e');
        c = -1;
        c_token = T_FALSE;

        return;
    } else if (c == 'n') {
        parser_match_char(header, 'u');
        parser_match_char(header, 'l');
        parser_match_char(header, 'l');
        c = -1;
        c_token = T_NONE;

        return;
    }

    //log_file(LogMessage, "Parser", "Token: %c", c);
    c_token = c;
    c = -1;
}

