/*
 * parser_json.c
 *
 */

#include "parser_json.h"
#include "chroma-engine.h"
#include "log.h"
#include "parser_internal.h"

static char buf[PARSE_BUF_SIZE];
static int buf_ptr = 0;

static int c_token;
static char c_value[PARSE_BUF_SIZE];

static void parser_json_parse_node(JSONNode *node, int socket_client);
static void parser_json_parse_object(JSONObject *obj, int socket_client);
static void parser_json_parse_array(JSONArray *array, int socket_client);

static void parser_match_token(int t, int socket_client);
static void parser_match_char(int socket_client, char c2);
static void parser_json_next_token(int socket_client);

void parser_json_free_node(JSONNode *node) {
    if (node->type == JSON_ARRAY) {
        JSONArray *array = &node->array;

        while (array->head.next != &array->tail) {
            JSONArrayNode *n = array->head.next;
            parser_json_free_node(n->node);
            REMOVE_NODE(n);
        }
    } else if (node->type == JSON_OBJECT) {
        JSONObject *obj = &node->object;

        while (obj->head.next != &obj->tail) {
            JSONAttributeNode *attr = obj->head.next;
            parser_json_free_node(attr->node);
            REMOVE_NODE(attr);
        }
    }

    free(node);
}

JSONNode *parser_json_attribute(JSONObject *node, const char *name) {
    for (JSONAttributeNode *attr = node->head.next; attr != &node->tail; attr = attr->next) {
        if (strncmp(attr->name, name, MAX_NAME_LENGTH)) {
            continue;
        }

        return attr->node;
    }

    return NULL;
}

int parser_json_get_int(JSONObject *obj, char *name) {
    JSONNode *node = parser_json_attribute(obj, name);
    if (node == NULL || node->type != JSON_INT) {
        log_file(LogWarn, "Parser", "Error: %s is not a JSON Integer", name);
        return 0;
    }

    return node->integer;
}

char *parser_json_get_string(JSONObject *obj, char *name) {
    JSONNode *node = parser_json_attribute(obj, name);
    if (node == NULL || node->type != JSON_STRING) {
        log_file(LogWarn, "Parser", "Error: %s is not a JSON String", name);
        return NULL;
    }

    return node->string;
}

float parser_json_get_float(JSONObject *obj, char *name) {
    JSONNode *node = parser_json_attribute(obj, name);
    if (node == NULL || node->type != JSON_FLOAT) {
        log_file(LogWarn, "Parser", "Error: %s is not a JSON Float", name);
        return 0.0;
    }

    return node->f;
}

unsigned char parser_json_get_bool(JSONObject *obj, char *name) {
    JSONNode *node = parser_json_attribute(obj, name);
    if (node == NULL || node->type != JSON_BOOL) {
        log_file(LogWarn, "Parser", "Error: %s is not a JSON Bool", name);
        return 0;
    }

    return node->boolean;
}

JSONNode *parser_receive_json(int socket_client) {
    JSONNode *root = NEW_STRUCT(JSONNode);
    parser_clean_buffer(&buf_ptr, buf);
    parser_json_next_token(socket_client);

    parser_json_parse_node(root, socket_client);

    return root;
}

static void parser_json_parse_node(JSONNode *node, int socket_client) {
    switch (c_token) {
        case T_NONE:
            parser_json_next_token(socket_client);
            node->type = JSON_NULL;
            break;

        case T_STRING:
            node->type = JSON_STRING;
            
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "String %s", c_value);
            }

            memset(node->string, '\0', sizeof node->string);
            memcpy(node->string, c_value, sizeof node->string);
            parser_json_next_token(socket_client);
            break;

        case T_INT:
            node->type = JSON_INT;
            node->integer = atoi(c_value);
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "Int %d", node->integer);
            }

            parser_json_next_token(socket_client);
            break;

        case T_FLOAT:
            node->type = JSON_FLOAT;
            node->f = atof(c_value);
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "Float %f", node->f);
            }

            parser_json_next_token(socket_client);
            break;

        case T_FALSE:
            node->type = JSON_BOOL;
            node->boolean = 0;
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "Bool False");
            }

            parser_json_next_token(socket_client);
            break;

        case T_TRUE:
            node->type = JSON_BOOL;
            node->boolean = 1;
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "Bool True");
            }

            parser_json_next_token(socket_client);
            break;

        case '{':
            node->type = JSON_OBJECT;
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "JSON Object Start");
            }

            parser_json_next_token(socket_client);
            parser_json_parse_object(&node->object, socket_client);
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "JSON Object End");
            }

            parser_match_token('}', socket_client);
            break;

        case '[':
            node->type = JSON_ARRAY;
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "JSON Array Start");
            }

            parser_json_next_token(socket_client);
            parser_json_parse_array(&node->array, socket_client);
            if (LOG_JSON) {
                log_file(LogMessage, "Parser", "JSON Array End");
            }

            parser_match_token(']', socket_client);
            break;

        case T_EOM:
            log_file(LogMessage, "Parser", "End of json");
            break;

        default:
            log_file(LogError, "Parser", "Unknown json token %c", c_token); 
    }
}

static void parser_json_parse_array(JSONArray *array, int socket_client) {
    array->head.next = &array->tail;
    array->head.prev = NULL;
    array->tail.next = NULL;
    array->tail.prev = &array->head;

    while (c_token != ']') {
        JSONArrayNode *array_node = NEW_STRUCT(JSONArrayNode);
        array_node->node = NEW_STRUCT(JSONNode);
        INSERT_BEFORE(array_node, &array->tail);
        parser_json_parse_node(array_node->node, socket_client);

        if (c_token == ',') {
            parser_match_token(',', socket_client);
        }
    }
}

static void parser_json_parse_object(JSONObject *obj, int socket_client) {
    JSONAttributeNode *attr;
    obj->head.next = &obj->tail;
    obj->head.prev = NULL;
    obj->tail.next = NULL;
    obj->tail.prev = &obj->head;

    while (c_token != '}') {
        attr = NEW_STRUCT(JSONAttributeNode);
        attr->node = NEW_STRUCT(JSONNode);
        INSERT_AFTER(attr, &obj->head);

        memset(attr->name, '\0', sizeof attr->name);
        memcpy(attr->name, c_value, sizeof attr->name);

        parser_match_token(T_STRING, socket_client);
        parser_match_token(':', socket_client);
        parser_json_parse_node(attr->node, socket_client);

        if (c_token == ',') {
            parser_match_token(',', socket_client);
        }
    }
}

static void parser_match_token(int t, int socket_client) {
    if (t == c_token) {
        parser_json_next_token(socket_client);
    } else {
        parser_incorrect_token(t, c_token, buf, buf_ptr);
    }
}

static void parser_match_char(int socket_client, char c2) {
    char c1 = parser_get_char(socket_client, &buf_ptr, buf);

    if (c1 == c2) {
        return ;
    } 

    log_file(LogError, "Parser", "Couldn't match char %c to char %c", c1, c2);
}

static void parser_json_next_token(int socket_client) {
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
        c_token = T_STRING;
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
        c_token = T_STRING;
        //log_file(LogMessage, "Parser", "String: %s", c_value);
        return;
    } else if ((c >= '0' && c <= '9') || c == '-') {
        // number
        c_token = T_INT;

        while (1) {
            c_value[i++] = c;
            c = parser_get_char(socket_client, &buf_ptr, buf);

            if (c == '.') {
                c_token = T_FLOAT;
            } else if (c < '0' || c > '9') {
                //log_file(LogMessage, "Parser", "Number: %s", c_value);
                return;
            }
        }
    } else if (c == 't' || c == 'T') {
        parser_match_char(socket_client, 'r');
        parser_match_char(socket_client, 'u');
        parser_match_char(socket_client, 'e');
        c = -1;
        c_token = T_TRUE;

        return;
    } else if (c == 'f' || c == 'F') {
        parser_match_char(socket_client, 'a');
        parser_match_char(socket_client, 'l');
        parser_match_char(socket_client, 's');
        parser_match_char(socket_client, 'e');
        c = -1;
        c_token = T_FALSE;

        return;
    } else if (c == 'n') {
        parser_match_char(socket_client, 'u');
        parser_match_char(socket_client, 'l');
        parser_match_char(socket_client, 'l');
        c = -1;
        c_token = T_NONE;

        return;
    }

    //log_file(LogMessage, "Parser", "Token: %c", c);
    c_token = c;
    c = -1;
}

