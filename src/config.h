/*
 * config.h
 */

#ifndef CONFIG
#define CONFIG

#include <stdint.h>

#define DEFAULT_CONFIG_PATH     "config/config.toml"

typedef struct {
    char *hub_addr;
    int hub_port;
} Config;

void config_parse_file(Config *c, char *file_name);

#endif // !CONFIG
