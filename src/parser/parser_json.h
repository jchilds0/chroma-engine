/*
 * parser_json.h
 *
 */

#ifndef PARSER_JSON
#define PARSER_JSON

#include "chroma-engine.h"
#include <stdio.h>
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
    JSON_TRUE,
    JSON_FALSE,
} JSONType;

typedef struct {
    size_t start;
    size_t num_items;
} JSONArray;

typedef struct {
    JSONType type;
    char name[MAX_NAME_LENGTH];

    union {
        JSONArray array;

        char string[MAX_NAME_LENGTH];
        int integer;
        float f;
    };
} JSONNode;

typedef struct {
    size_t count;
    size_t capacity;
    size_t *items;
} JSONIndices;

typedef struct {
    size_t   count;
    size_t   capacity;
    JSONNode *items;
    JSONIndices objects;
} JSONArena;

extern JSONArena json_arena;

JSONNode *parser_receive_json(int socket_client);
void parser_clean_json(void);

JSONNode *parser_json_attribute(JSONNode *obj, const char *attr);
int parser_json_get_int(JSONNode *obj, char *name);
char *parser_json_get_string(JSONNode *obj, char *name);
float parser_json_get_float(JSONNode *obj, char *name);
unsigned char parser_json_get_bool(JSONNode *obj, char *name);

#endif // !PARSER_JSON
