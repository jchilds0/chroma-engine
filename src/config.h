/*
 * config.h
 */

#ifndef CONFIG
#define CONFIG

#include <stdint.h>

typedef struct {
    char *hub_addr;
    int hub_port;
    int engine_port;
} Config;

void config_parse_file(Config *c, char *file_name);

#endif // !CONFIG
