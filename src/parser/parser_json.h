/*
 * parser_json.h
 *
 */

#ifndef PARSER_JSON
#define PARSER_JSON

#include "chroma-engine.h"

#define MAX_NAME_LENGTH     128

typedef enum {
    T_NONE,
    T_STRING,
    T_INT,
    T_FLOAT,
    T_FALSE,
    T_TRUE,
    T_EOM,
} JSONToken;

typedef enum {
    JSON_NULL,
    JSON_OBJECT,
    JSON_ARRAY,

    JSON_STRING,
    JSON_INT,
    JSON_FLOAT,
    JSON_BOOL,
} JSONType;

typedef struct JSONAttributeNode {
    char name[MAX_NAME_LENGTH];
    struct JSONNode *node;

    struct JSONAttributeNode *next;
    struct JSONAttributeNode *prev;
} JSONAttributeNode;

typedef struct {
    JSONAttributeNode head;
    JSONAttributeNode tail;
} JSONObject;

typedef struct JSONArrayNode {
    struct JSONNode *node;

    struct JSONArrayNode *next;
    struct JSONArrayNode *prev;
} JSONArrayNode;

typedef struct {
    JSONArrayNode head;
    JSONArrayNode tail;
} JSONArray;

typedef struct JSONNode {
    JSONType type;

    union {
        JSONObject object;
        JSONArray array;

        char string[MAX_NAME_LENGTH];
        int integer;
        unsigned char boolean;
        float f;
    };
} JSONNode;

JSONNode *parser_receive_json(int socket_client);
void parser_json_free_node(JSONNode *node);

JSONNode *parser_json_attribute(JSONObject *node, const char *attr);
int parser_json_get_int(JSONObject *obj, char *name);
char *parser_json_get_string(JSONObject *obj, char *name);
float parser_json_get_float(JSONObject *obj, char *name);
unsigned char parser_json_get_bool(JSONObject *obj, char *name);

#endif // !PARSER_JSON
