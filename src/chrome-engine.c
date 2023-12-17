/*
 * chroma-engine.c 
 */

#include "chroma-typedefs.h"
#include "chroma-prototypes.h"
#include "log.h"
#include "graphics.h"

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

Engine engine;

int main(int argc, char **argv) {
    uint wid;
    log_start(-1);

    // Graphics Hub 
    engine.hub = graphics_new_graphics_hub();
    //graphics_load_from_file(engine.hub, file);

    switch (argc) {
    case 1:
        log_start(LogEngine);
        engine_window();

        break;
    case 3:
        if (strcmp(argv[1], "-wid") == 0) {
            log_start(LogPreview);
            wid = atoi(argv[2]);
            log_file(LogMessage, "Engine", "Recieved wid %d", wid);
            preview_window(wid);
            break;
        }

        log_file(LogError, "Engine", "Incorrect argument %s", argv[1]);
        return 0;
    default: 
        log_file(LogError, "Engine", "Incorrect usage");
        return 0;
    }

    free_engine();
    return 1;
}


void free_engine(void) {
    shutdown(engine.socket, SHUT_RDWR);
    graphics_free_graphics_hub(engine.hub);
    log_file(LogMessage, "Engine", "Shutdown");
    exit(1);
}
