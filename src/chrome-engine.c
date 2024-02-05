/*
 * chroma-engine.c 
 */

#include "chroma-typedefs.h"
#include "chroma-prototypes.h"
#include "log.h"
#include "parser.h"

#include <bits/getopt_core.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

Engine engine;

int main(int argc, char **argv) {
    int x, wid, 
        port = 9000;
    int hflag = 0;
    int wflag = 0;
    char *wval, 
        *gval = "127.0.0.1";
    opterr = 0;
    log_start(-1);

    while ((x = getopt(argc, argv, "g:hw:")) != -1) {
        switch (x) {
            case 'g':
                gval = optarg;
                break;
            case 'h':
                hflag = 1;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'w':
                wval = optarg;
                wflag = 1;
                break;
            default:
                log_file(LogError, "Engine", "Invalid Args");
        }
    }

    if (hflag) {
        printf("Usage:\n");
        printf("  -g [addr]\tGraphics hub address (default: 127.0.0.1) \n");
        printf("  -p [addr]\tGraphics hub port (default: 9000)\n");
        printf("  -w [id]\tUse preview mode with gtk plug id [id]\n");
        return 0;
    }

    engine.hub_socket = parser_tcp_start_client(gval, port);
    log_file(LogMessage, "Engine", "Graphics hub %s:%d", gval, port); 
    parser_parse_hub(&engine);

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

