/*
 * parser_http.h
 *
 */

#ifndef PARSER_HTTP
#define PARSER_HTTP

#include "parser_internal.h"

typedef enum {
    CHUNKED,
    COMPRESS,
    DEFLATE,
    GZIP,
} TransferEncoding;

typedef struct HeaderNode {
    char name[PARSE_BUF_SIZE];
    char value[PARSE_BUF_SIZE];

    struct HeaderNode *next;
    struct HeaderNode *prev;
} HeaderNode;

typedef struct {
    int socket_client;
    int transfer_encoding;
    int content_length;
    int chunk_size;
    int read;

    HeaderNode head;
    HeaderNode tail;
} HTTPHeader;

int         parser_http_get_bytes(HTTPHeader *header, int *buf_ptr, char *buf);
int         parser_http_get_char(HTTPHeader *header, int *buf_ptr, char *buf, char *c);

int         parser_http_get(int socket_client, const char *addr);
HTTPHeader  *parser_http_new_header(int socket_client);
int         parser_http_header(HTTPHeader *header, int *buf_ptr, char *buf);
void        parser_http_free_header(HTTPHeader *header);

#endif // !PARSER_HTTP
