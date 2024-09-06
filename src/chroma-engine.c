/*
 * chroma-engine.c 
 */

#include "chroma-typedefs.h"
#include "chroma-prototypes.h"
#include "config.h"
#include "log.h"
#include "parser.h"

#include <bits/getopt_core.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

Engine engine;
Config config;

int main(int argc, char **argv) {
    int x, wid;
    int hflag = 0;
    int wflag = 0;
    int cflag = 0;

    char *wval;
    char *config_path;
    opterr = 0;

    config.hub_addr = NULL;

    log_start(-1);
    parser_init_sockets();

    while ((x = getopt(argc, argv, "c:hw:")) != -1) {
        switch (x) {
            case 'c':
                cflag = 1;
                config_path = optarg;
                break;

            case 'h':
                hflag = 1;
                break;

            case 'w':
                wval = optarg;
                wflag = 1;
                break;

            default:
                log_file(LogError, "Engine", "Invalid Args %d", x);
        }
    }

    if (hflag) {
        printf("Usage:\n");
        printf("  -c [file]\tConfig File\n");
        printf("  -w [id]\tUse preview mode with gtk plug id [id]\n");
        return 0;
    }

    if (cflag) {
        config_parse_file(&config, config_path);
    } else {
        log_file(LogMessage, "Config", "No config file specified, loading default " DEFAULT_CONFIG_PATH);
        config_parse_file(&config, DEFAULT_CONFIG_PATH);
    }

    engine.hub_socket = parser_tcp_start_client(config.hub_addr, config.hub_port);
    log_file(LogMessage, "Engine", "Graphics hub %s:%d", config.hub_addr, config.hub_port); 

    clock_t start, end;
    start = clock();

    parser_parse_hub(&engine);

    end = clock();

    log_file(LogMessage, "Parser", "Imported Chroma Hub in %f ms", ((double) (end - start) * 1000) / CLOCKS_PER_SEC);

    if (wflag) {
        // Preview process
        log_start(LogPreview);
        wid = atoi(wval);
        log_file(LogMessage, "Engine", "Recieved wid %d", wid);
        preview_window(wid);
    } else {
        // Engine process
        log_start(LogEngine);
        engine_window();
    }

    return 1;
}

