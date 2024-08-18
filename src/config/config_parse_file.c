/*
 * config_parse_file.c
 */

#include "chroma-engine.h"
#include "config.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_LINE_MAX_LEN     1024
#define LOG_CONFIG_PARSE      1

typedef enum {
    NONE,
    ENGINE,
} Table;

typedef enum {
    TABLE_LINE,
    FIELD_LINE,
    BLANK_LINE,
} LineType;

static int is_whitespace(char c) {
    return c == '\t' || c == ' ' || c == '\n';
}

static LineType config_line_type(const char *buf) {
    int i = 0;

    while (is_whitespace(buf[i])) {
        i++;
    }

    switch (buf[i]) {
        case '[':
            while (buf[++i] != '\0') {
                if (buf[i] == ']') {
                    return TABLE_LINE;
                }
            }

            log_file(LogMessage, "Config", buf);
            log_file(LogError, "Config", "Invalid category format");
            break;

        case ' ':
        case '\t':
        case '\n':
        case '-':
        case '#':
            return BLANK_LINE;

        default:
            return FIELD_LINE;
    }

    return BLANK_LINE;
}

static Table config_parse_category(const char *buf) {
    char name[FILE_LINE_MAX_LEN];
    int i = 0;

    while (buf[i++] != '[');

    for (int j = 0; buf[i + j] != ']'; j++) {
        name[j] = buf[i + j];
    }

    if (LOG_CONFIG_PARSE) {
        log_file(LogMessage, "Config", "Table: %s", name);
    }

    if (strncmp(name, "Engine", sizeof name) == 0) {
        return ENGINE;
    } else {
        log_file(LogWarn, "Config", "Unknown table %s", name);
        return NONE;
    }
}

static void config_parse_field(Config *c, Table t, const char *buf) {
    char field[FILE_LINE_MAX_LEN];
    char value[FILE_LINE_MAX_LEN];
    int i = 0;

    memset(field, '\0', sizeof field);
    memset(value, '\0', sizeof value);

    while (is_whitespace(buf[i])) {
        i++;
    }

    for (int j = 0; j < FILE_LINE_MAX_LEN; j++) {
        if (is_whitespace(buf[i])) {
            break;
        }
        
        field[j] = buf[i];
        i++;
    }

    while (is_whitespace(buf[i])) {
        i++;
    }

    if (buf[i++] != '=') {
        log_file(LogMessage, "Config", buf);
        log_file(LogWarn, "Config", "Missing '=' between field and value");
        return;
    }

    while (is_whitespace(buf[i])) {
        i++;
    }

    if (buf[i] == '"') {
        i++;
        for (int j = 0; buf[i + j] != '"'; j++) {
            value[j] = buf[i + j];
        }
    } else {
        for (int j = 0; !is_whitespace(buf[i + j]); j++) {
            value[j] = buf[i + j];
        }
    }

    if (LOG_CONFIG_PARSE) {
        log_file(LogMessage, "Config", "\tField: %s, Value: %s", field, value);
    }

    switch (t) {
        case ENGINE:
            if (strcmp(field, "ChromaHubAddr") == 0) {

                int addr_len = strlen(value) + 1;
                c->hub_addr = NEW_ARRAY(addr_len, char);
                memcpy(c->hub_addr, value, addr_len);

            } else if (strcmp(field, "ChromaHubPort") == 0) {

                c->hub_port = atoi(value);

            } else {

                log_file(LogMessage, "Config", buf);
                log_file(LogWarn, "Config", "Unknown field %s", field);

            }
            break;
        
        default:
            log_file(LogWarn, "Config", "Unknown table %d", t);
    }
}

void config_parse_file(Config *c, char *file_name) {
    FILE *file = fopen(file_name, "r");

    if (file == NULL) {
        log_file(LogError, "Config", "Error while opening config file %s", file_name);
    }

    char buf[FILE_LINE_MAX_LEN];
    memset(buf, '\0', sizeof buf);

    Table cat = NONE;
    LineType type = BLANK_LINE;
    while (fgets(buf, sizeof buf, file) != NULL) {
        type = config_line_type(buf);

        switch (type) {
            case TABLE_LINE:
                cat = config_parse_category(buf);
                break;

            case FIELD_LINE:
                config_parse_field(c, cat, buf);
                break;

            case BLANK_LINE:
                break;

            default:
                log_file(LogWarn, "Config", "Unknown line type %d", type);
        }

        memset(buf, '\0', sizeof buf);
    }
}

