/*
 * parser_json.c
 *
 */

#include "parser_json.h"
#include "parser_http.h"
#include "parser_internal.h"

JSONArena json_arena = {0, DA_INIT_CAPACITY, NULL, {0, DA_INIT_CAPACITY, NULL}};

static char buf[PARSE_BUF_SIZE];
static int buf_ptr = 0;

static int c_token;
static char c_value[PARSE_BUF_SIZE];

static int parser_json_parse_node(HTTPHeader *header, size_t *);
static int parser_json_parse_object(HTTPHeader *header, JSONNode *node);
static int parser_json_parse_array(HTTPHeader *header, JSONNode *node);

static int parser_match_token(HTTPHeader *header, int t);
static int parser_json_next_token(HTTPHeader *header);

JSONNode *parser_json_attribute(JSONNode *obj, const char *name) {
    if (obj == NULL || obj->type != JSON_OBJECT) {
        log_file(LogWarn, "Parser", "JSON node is not an object");
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
    if (node == NULL) {
        //log_file(LogWarn, "Parser", "No JSON String %s", name);
        return NULL;
    }

    if (node->type != JSON_STRING) {
        log_file(LogWarn, "Parser", "%s is not a JSON String: %d", name, node->type);
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

int parser_receive_json(int socket_client, JSONNode *root) {
    if (json_arena.items == NULL) {
        json_arena.items = NEW_ARRAY(DA_INIT_CAPACITY, JSONNode);
        json_arena.objects.items = NEW_ARRAY(DA_INIT_CAPACITY, size_t);
    }

    log_assert(json_arena.count == 0, "Parser", "JSON Arena was not cleaned up");
    parser_clean_buffer(&buf_ptr, buf);

    HTTPHeader *header = parser_http_new_header(socket_client);
    if (parser_http_header(header, &buf_ptr, buf) < 0) {
        log_file(LogError, "Parser", "parser_receive_json: %s %d", __FILE__, __LINE__);
        return -1;
    }

    if (parser_json_next_token(header) < 0) {
        log_file(LogError, "Parser", "parser_receive_json: %s %d", __FILE__, __LINE__);
        return -1;
    }

    size_t root_index;
    if (parser_json_parse_node(header, &root_index) < 0) {
        log_file(LogError, "Parser", "parser_receive_json: %s %d", __FILE__, __LINE__);
        return -1;
    }

    parser_http_free_header(header);
    *root = json_arena.items[root_index];
    return 0;
}

void parser_clean_json(void) {
    if (json_arena.items == NULL || json_arena.objects.items == NULL) {
        return;
    }

    json_arena.count = 0;
    json_arena.objects.count = 0;
}

static int parser_json_parse_node(HTTPHeader *header, size_t *index) {
    JSONNode node;
    memset(node.name, '\0', sizeof node.name);

    switch (c_token) {
        case T_NONE:
            node.type = JSON_NULL;
            if (parser_json_next_token(header) < 0) {
                log_file(LogError, "Parser", "parser_json_parse_node: %s %d", __FILE__, __LINE__);
                return -1;
            }
            break;

        case T_STRING:
            node.type = JSON_STRING;
            memset(node.string, '\0', sizeof node.string);
            memcpy(node.string, c_value, sizeof node.string);

            if (parser_json_next_token(header) < 0) {
                log_file(LogError, "Parser", "parser_json_parse_node: %s %d", __FILE__, __LINE__);
                return -1;
            }
            break;

        case T_INT:
            node.type = JSON_INT;
            node.integer = atoi(c_value);
            if (parser_json_next_token(header) < 0) {
                log_file(LogError, "Parser", "parser_json_parse_node: %s %d", __FILE__, __LINE__);
                return -1;
            }
            break;

        case T_FLOAT:
            node.type = JSON_FLOAT;
            node.f = atof(c_value);
            if (parser_json_next_token(header) < 0) {
                log_file(LogError, "Parser", "parser_json_parse_node: %s %d", __FILE__, __LINE__);
                return -1;
            }
            break;

        case T_FALSE:
            node.type = JSON_FALSE;
            if (parser_json_next_token(header) < 0) {
                log_file(LogError, "Parser", "parser_json_parse_node: %s %d", __FILE__, __LINE__);
                return -1;
            }
            break;

        case T_TRUE:
            node.type = JSON_TRUE;
            if (parser_json_next_token(header) < 0) {
                log_file(LogError, "Parser", "parser_json_parse_node: %s %d", __FILE__, __LINE__);
                return -1;
            }
            break;

        case '{':
            node.type = JSON_OBJECT;
            if (parser_json_parse_object(header, &node) < 0) {
                log_file(LogError, "Parser", "parser_json_parse_node: %s %d", __FILE__, __LINE__);
                return -1;
            };

            if (parser_match_token(header, '}') < 0) {
                log_file(LogError, "Parser", "parser_json_parse_node: %s %d", __FILE__, __LINE__);
                return -1;
            };

            break;

        case '[':
            node.type = JSON_ARRAY;
            if (parser_json_parse_array(header, &node) < 0) {
                log_file(LogError, "Parser", "parser_json_parse_node: %s %d", __FILE__, __LINE__);
                return -1;
            }

            if (parser_match_token(header, ']') < 0) {
                log_file(LogError, "Parser", "parser_json_parse_node: %s %d", __FILE__, __LINE__);
                return -1;
            };

            break;

        case T_EOM:
            log_file(LogMessage, "Parser", "End of json");
            break;

        default:
            log_file(LogError, "Parser", "Unknown json token %c", c_token); 
            return -1;
    }

    *index = json_arena.count;
    DA_APPEND(&json_arena, node);
    return 0;
}

static int parser_json_parse_array(HTTPHeader *header, JSONNode *node) {
    if (parser_match_token(header, '[') < 0) {
        log_file(LogError, "Parser", "parser_json_parse_array: %s %d", __FILE__, __LINE__);
        return -1;
    };

    JSONIndices indices;
    indices.count = 0;
    indices.capacity = 128;
    indices.items = NEW_ARRAY(indices.capacity, size_t);

    while (c_token != ']') {
        size_t item_index;
        if (parser_json_parse_node(header, &item_index) < 0) {
            log_file(LogError, "Parser", "parser_json_parse_array: %s %d", __FILE__, __LINE__);
            return -1;
        }
        DA_APPEND(&indices, item_index);
        if (c_token == ',' && parser_json_next_token(header) < 0) {
            log_file(LogError, "Parser", "parser_json_parse_array: %s %d", __FILE__, __LINE__);
            return -1;
        }
    }

    node->array.start = json_arena.objects.count;
    node->array.num_items = indices.count;

    for (int i = 0; i < indices.count; i++) {
        DA_APPEND(&json_arena.objects, indices.items[i]);
    }

    return 0;
}

static int parser_json_parse_object(HTTPHeader *header, JSONNode *node) {
    JSONIndices indices;
    char attr_name[MAX_NAME_LENGTH];
    indices.count = 0;
    indices.capacity = 128;
    indices.items = NEW_ARRAY(indices.capacity, size_t);

    if (parser_match_token(header, '{') < 0) {
        log_file(LogError, "Parser", "parser_json_parse_object: %s %d", __FILE__, __LINE__);
        return -1;
    }

    while (c_token != '}') {
        memset(attr_name, '\0', sizeof attr_name);
        memcpy(attr_name, c_value, sizeof attr_name);

        if (parser_match_token(header, T_STRING) < 0) {
            log_file(LogError, "Parser", "parser_json_parse_object: %s %d", __FILE__, __LINE__);
            return -1;
        }

        if (parser_match_token(header, ':') < 0) {
            log_file(LogError, "Parser", "parser_json_parse_object: %s %d", __FILE__, __LINE__);
            return -1;
        }

        size_t item_index;
        if (parser_json_parse_node(header, &item_index) < 0) {
            log_file(LogError, "Parser", "parser_json_parse_object: %s %d", __FILE__, __LINE__);
            return -1;
        }

        DA_APPEND(&indices, item_index);

        char *name = json_arena.items[item_index].name;
        memcpy(name, attr_name, sizeof attr_name);

        if (c_token == ',' && parser_json_next_token(header) < 0) {
            log_file(LogError, "Parser", "parser_json_parse_object: %s %d", __FILE__, __LINE__);
            return -1;
        }
    }

    node->array.start = json_arena.objects.count;
    node->array.num_items = indices.count;

    for (int i = 0; i < indices.count; i++) {
        DA_APPEND(&json_arena.objects, indices.items[i]);
    }

    return 0;
}

static int parser_match_token(HTTPHeader *header, int t) {
    if (t != c_token) {
        parser_incorrect_token(t, c_token, buf_ptr, buf);
        return -1;
    }

    return parser_json_next_token(header);
}

static unsigned char parser_match_char(HTTPHeader *header, char c2) {
    char c1;

    if (parser_http_get_char(header, &buf_ptr, buf, &c1) < 0) {
        return 0;
    }

    if (c1 != c2) {
        log_file(LogError, "Parser", "Couldn't match char %c to char %c", c1, c2);
    }

    return c1 == c2;
}

static int parser_json_next_token(HTTPHeader *header) {
    static char c = -1;
    int i = 0;
    memset(c_value, '\0', PARSE_BUF_SIZE);

    if (c == -1) {
        if (parser_http_get_char(header, &buf_ptr, buf, &c) < 0) {
            log_file(LogError, "Parser", "parser_json_next_token: %s %d", __FILE__, __LINE__);
            return -1;
        }
    }

    // skip whitespace
    while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        if (parser_http_get_char(header, &buf_ptr, buf, &c) < 0) {
            log_file(LogError, "Parser", "parser_json_next_token: %s %d", __FILE__, __LINE__);
            return -1;
        }
    }

    if (c == '\'') {
        // attr 
        while (1) {
            if (parser_http_get_char(header, &buf_ptr, buf, &c) < 0) {
                log_file(LogError, "Parser", "parser_json_next_token: %s %d", __FILE__, __LINE__);
                return -1;
            }

            if (c == '\'') {
                break;
            }

            if (i >= PARSE_BUF_SIZE) {
                log_file(LogError, "Parser", "Parser ran out of memory");
                return -1;
            }

            c_value[i++] = c;
        } 

        c = -1;
        c_token = T_STRING;
        //log_file(LogMessage, "Parser", "String: %s", c_value);

    } else if (c == '"') {
        // attr 
        while (1) {
            if (parser_http_get_char(header, &buf_ptr, buf, &c) < 0) {
                log_file(LogError, "Parser", "parser_json_next_token: %s %d", __FILE__, __LINE__);
                return -1;
            }

            if (c == '\"') {
                break;
            }

            if (i >= PARSE_BUF_SIZE) {
                log_file(LogError, "Parser", "Parser ran out of memory");
                return -1;
            }

            c_value[i++] = c;
        }

        c = -1;
        c_token = T_STRING;
        //log_file(LogMessage, "Parser", "String: %s", c_value);

    } else if ((c >= '0' && c <= '9') || c == '-') {
        // number
        c_token = T_INT;

        while (1) {
            c_value[i++] = c;
            if (parser_http_get_char(header, &buf_ptr, buf, &c) < 0) {
                log_file(LogError, "Parser", "parser_json_next_token: %s %d", __FILE__, __LINE__);
                return -1;
            }

            if (c == '.') {
                c_token = T_FLOAT;
            } else if (c < '0' || c > '9') {
                //log_file(LogMessage, "Parser", "Number: %s", c_value);
                break;
            }
        }
    } else if (c == 't' || c == 'T') {
        if (!parser_match_char(header, 'r')) {
            return -1;
        }

        if (!parser_match_char(header, 'u')) {
            return -1;
        }

        if (!parser_match_char(header, 'e')) {
            return -1;
        }

        c = -1;
        c_token = T_TRUE;
    } else if (c == 'f' || c == 'F') {
        if (!parser_match_char(header, 'a')) {
            return -1;
        }

        if (!parser_match_char(header, 'l')) {
            return -1;
        }

        if (!parser_match_char(header, 's')) {
            return -1;
        }

        if (!parser_match_char(header, 'e')) {
            return -1;
        }

        c = -1;
        c_token = T_FALSE;
    } else if (c == 'n') {
        if (!parser_match_char(header, 'u')) {
            return -1;
        }

        if (!parser_match_char(header, 'l')) {
            return -1;
        }

        if (!parser_match_char(header, 'l')) {
            return -1;
        }

        c = -1;
        c_token = T_NONE;
    } else {
        c_token = c;
        c = -1;
    }

    //log_file(LogMessage, "Parser", "Token: %c", c);
    return 0;
}

