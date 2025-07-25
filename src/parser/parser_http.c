/*
 * parser_http.c
 *
 */

#include "chroma-macros.h"
#include "log.h"
#include "parser_internal.h"
#include "parser_json.h"
#include "parser_http.h"
#include <sys/socket.h>

int parser_http_get(int socket_client, const char *addr) {
    char msg[PARSE_BUF_SIZE];
    memset(msg, '\0', sizeof msg);

    sprintf(msg, "GET http://%s HTTP/1.1\nHost: ChromaViz\n\n", addr);
    if (send(socket_client, msg, strlen(msg), 0) < 0) {
        log_file(LogError, "Parser", "Error requesting graphics hub"); 
        return -1;
    }

    return 0;
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

int parser_http_header(HTTPHeader *header, int *buf_ptr, char *buf) {
    char c = '\0', line[PARSE_BUF_SIZE];
    int i = 0;

    // status
    while (c != '\n') {
        if (parser_get_char(header->socket_client, buf_ptr, buf, &c) < 0) {
            log_file(LogError, "Parser", "parser_http_header: %s %d", __FILE__, __LINE__);
            return -1;
        }

        line[i++] = c;
    }

    // attrs
    while (1) {
        HeaderNode *node = NEW_STRUCT(HeaderNode);
        memset(node->name, '\0', sizeof node->name);
        memset(node->value, '\0', sizeof node->value);

        i = 0;

        // name
        while (1) {
            if (parser_get_char(header->socket_client, buf_ptr, buf, &c) < 0) {
                log_file(LogError, "Parser", "parser_http_header: %s %d", __FILE__, __LINE__);
                return -1;
            }

            if (c == ':' || c == '\r') {
                break;
            }

            if (i == PARSE_BUF_SIZE) {
                log_file(LogWarn, "Parser", "HTTP attr name too long");
                break;
            }

            node->name[i++] = c;
        };

        if (parser_get_char(header->socket_client, buf_ptr, buf, &c) < 0) {
            log_file(LogError, "Parser", "parser_http_header: %s %d", __FILE__, __LINE__);
            return -1;
        }

        // end of header
        if (i == 0) {
            break;
        }

        i = 0;

        // value
        while (1) {
            if (parser_get_char(header->socket_client, buf_ptr, buf, &c) < 0) {
                log_file(LogError, "Parser", "parser_http_header: %s %d", __FILE__, __LINE__);
                return -1;
            }

            if (c == '\r') {
                break;
            }

            if (i == PARSE_BUF_SIZE) {
                log_file(LogWarn, "Parser", "HTTP attr value too long");
                break;
            }

            node->value[i++] = c;
        };

        if (parser_get_char(header->socket_client, buf_ptr, buf, &c) < 0) {
            log_file(LogError, "Parser", "parser_http_header: %s %d", __FILE__, __LINE__);
            return -1;
        }

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

    return 0;
}

int parser_http_get_bytes(HTTPHeader *header, int *buf_ptr, char *buf) {
    char c_temp = 0;
    if (header->transfer_encoding == CHUNKED && header->read == header->chunk_size) {
        // end of a chunk, read next chunk size
        char line[PARSE_BUF_SIZE];
        memset(line, '\0', PARSE_BUF_SIZE);
        int i = 0;

        while (c_temp != '\r') {
            if (parser_get_char(header->socket_client, buf_ptr, buf, &c_temp) < 0) {
                log_file(LogError, "Parser", "parser_http_get_bytes: %s %d", __FILE__, __LINE__);
                return -1;
            }

            line[i++] = c_temp;
        }
        if (parser_get_char(header->socket_client, buf_ptr, buf, &c_temp) < 0) {
            log_file(LogError, "Parser", "parser_http_get_bytes: %s %d", __FILE__, __LINE__);
            return -1;
        }

        header->read = 0;
        header->chunk_size = strtol(line, NULL, 16);
        if (LOG_PARSER) {
            log_file(LogMessage, "Parser", "Chunk Size: %d", header->chunk_size);
        }
    } 

    if (*buf_ptr == PARSE_BUF_SIZE) {
        parser_clean_buffer(buf_ptr, buf);
        if (parser_tcp_recieve_message(header->socket_client, buf) != SERVER_MESSAGE) {
            return -1;
        }
    }

    if (header->transfer_encoding == CHUNKED) {
        return MIN(header->chunk_size - header->read, PARSE_BUF_SIZE - *buf_ptr);
    } else {
        return MIN(header->content_length - header->read, PARSE_BUF_SIZE - *buf_ptr);
    }

}

int parser_http_get_char(HTTPHeader *header, int *buf_ptr, char *buf, char *c) {
    int length = parser_http_get_bytes(header, buf_ptr, buf);
    if (length < 0) {
        log_file(LogError, "Parser", "parser_http_get_char: %s %d", __FILE__, __LINE__);
        return -1;
    } else if (length == 0) {
        *c = T_NONE;
        return 0;
    }


    header->read++;
    if (parser_get_char(header->socket_client, buf_ptr, buf, c) < 0) {
        log_file(LogError, "Parser", "parser_http_get_char: %s %d", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

