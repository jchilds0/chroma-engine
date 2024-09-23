/*
 * parser_http.c
 *
 */

#include "chroma-engine.h"
#include "log.h"
#include "parser/parser_internal.h"
#include "parser/parser_json.h"
#include "parser_http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

void parser_http_get(int socket_client, const char *addr) {
    char msg[PARSE_BUF_SIZE];
    memset(msg, '\0', sizeof msg);

    sprintf(msg, "GET http://%s HTTP/1.1\nHost: ChromaViz\n\n", addr);
    if (send(socket_client, msg, strlen(msg), 0) < 0) {
        log_file(LogError, "Parser", "Error requesting graphics hub"); 
    }
}

HTTPHeader *parser_http_new_header(int socket_client) {
    HTTPHeader *header = NEW_STRUCT(HTTPHeader);
    header->socket_client = socket_client;
    header->transfer_encoding = -1;
    header->content_length = -1;
    header->chunk_size = 0;
    header->read = 0;

    header->head.next = (struct HeaderNode *)&header->tail;
    header->head.prev = NULL;
    header->tail.next = NULL;
    header->tail.prev = (struct HeaderNode *)&header->head;

    return header;
}

void parser_http_free_header(HTTPHeader *header) {
    if (header == NULL) {
        return;
    }

    while (header->head.next != &header->tail) {
        HeaderNode *node = header->head.next;
        REMOVE_NODE(node);
        free(node);
    }

    free(header);
}

void parser_http_header(HTTPHeader *header, int *buf_ptr, char *buf) {
    char c, line[PARSE_BUF_SIZE];
    int i = 0;

    // status
    while ((line[i++] = parser_get_char(header->socket_client, buf_ptr, buf)) != '\n');

    // attrs
    while (1) {
        HeaderNode *node = NEW_STRUCT(HeaderNode);
        memset(node->name, '\0', sizeof node->name);
        memset(node->value, '\0', sizeof node->value);

        i = 0;

        // name
        while (1) {
            c = parser_get_char(header->socket_client, buf_ptr, buf);
            if (c == ':' || c == '\r') {
                break;
            }

            if (i == PARSE_BUF_SIZE) {
                log_file(LogWarn, "Parser", "HTTP attr name too long");
                break;
            }

            node->name[i++] = c;
        };

        parser_get_char(header->socket_client, buf_ptr, buf);

        // end of header
        if (i == 0) {
            break;
        }

        i = 0;

        // value
        while (1) {
            c = parser_get_char(header->socket_client, buf_ptr, buf);
            if (c == '\r') {
                break;
            }

            if (i == PARSE_BUF_SIZE) {
                log_file(LogWarn, "Parser", "HTTP attr value too long");
                break;
            }

            node->value[i++] = c;
        };

        parser_get_char(header->socket_client, buf_ptr, buf);

        if (LOG_PARSER) {
            log_file(LogMessage, "Parser", "\tName: '%s'", node->name);
            log_file(LogMessage, "Parser", "\tValue: '%s'", node->value);
        }

        INSERT_AFTER(node, &header->head);
    }

    for (HeaderNode *node = header->head.next; node != &header->tail; node = node->next) {
        if (strncmp(node->name, "Content-Length", PARSE_BUF_SIZE) == 0) {

            header->content_length = atoi(node->value);

        } else if (strncmp(node->name, "Transfer-Encoding", PARSE_BUF_SIZE) == 0) {

            if (strncmp(node->value, "chunked", PARSE_BUF_SIZE) == 0) {
                header->transfer_encoding = CHUNKED;
            } else if (strncmp(node->value, "compress", PARSE_BUF_SIZE) == 0) {
                header->transfer_encoding = COMPRESS;

                log_file(LogError, "Parser", "Compress not implemented");
            } else if (strncmp(node->value, "deflate", PARSE_BUF_SIZE) == 0) {
                header->transfer_encoding = DEFLATE;

                log_file(LogError, "Parser", "Deflate not implemented");
            } else if (strncmp(node->value, "gzip", PARSE_BUF_SIZE) == 0) {
                header->transfer_encoding = GZIP;

                log_file(LogError, "Parser", "gzip not implemented");
            } else {
                log_file(LogWarn, "Parser", "Unknown transfer encoding: %s", node->value);
            }

        }
    }
}

char parser_http_get_char(HTTPHeader *header, int *buf_ptr, char *buf) {
    if (header->transfer_encoding == CHUNKED && header->chunk_size == header->read) {
        // end of a chunk, read next chunk size
        char line[PARSE_BUF_SIZE];
        memset(line, '\0', PARSE_BUF_SIZE);
        int i = 0;

        while ((line[i++] = parser_get_char(header->socket_client, buf_ptr, buf)) != '\r');
        parser_get_char(header->socket_client, buf_ptr, buf);

        header->chunk_size = strtol(line, NULL, 16);
        if (LOG_PARSER) {
            log_file(LogMessage, "Parser", "Chunk Size: %d", header->chunk_size);
        }

    } else if (header->content_length == header->read) {
        log_file(LogMessage, "Parser", "End of content");
        return T_NONE;
    }

    header->read++;
    return parser_get_char(header->socket_client, buf_ptr, buf);
}

