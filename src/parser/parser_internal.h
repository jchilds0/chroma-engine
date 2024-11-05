/*
 * parser_internal.h
 *
 * Header file for the parser module source 
 * code. Should not be included by any source 
 * files outside the /parser dir.
 *
 */

#ifndef PARSER_INTERNAL
#define PARSER_INTERNAL

#include "chroma-typedefs.h"
#include "geometry.h"

#define MAX_CONNECTIONS     10
#define LOG_PARSER          0
#define LOG_TEMPLATE        0

// ServerResponse MUST BE < 0 otherwise socket_client in parser will be incorrect
typedef enum {
    SERVER_MESSAGE = -3,
    SERVER_TIMEOUT,
    SERVER_CLOSE,
} ServerResponse;

typedef enum {
    NONE,

    /* header tokens */
    VERSION,
    LAYER,
    ACTION,
    TEMPID,

    /* other tokens */
    ATTR,
    VALUE,
    EOM,

    /* hub */ 
    STRING, 
    INT,
    FLOAT,
    BOOL,
} Token;

ServerResponse  parser_tcp_recieve_message(int socket_client, char *buf);
ServerResponse  parser_recieve_image(Engine *eng, GeometryImage *img);

// parser_http.c
int             parser_update_template(Engine *eng, int page_num);

// parser_util.c
void            parser_incorrect_token(char tok1, char tok2, int buf_ptr, char *buf);
int             parser_get_char(int socket_client, int *buf_ptr, char *buf, char *c);
void            parser_clean_buffer(int *buf_ptr, char *buf);
ServerResponse  parser_get_message(int socket_client, int *buf_ptr, char *buf);


#endif // !PARSER_INTERNAL

