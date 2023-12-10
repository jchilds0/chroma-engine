/*
 * chroma-engine.c 
 */

#include "chroma-engine.h"
#include <sys/socket.h>

Engine engine;

int main(int argc, char **argv) {
    uint wid;
    log_start(-1);

    // Graphics Hub 
    engine.hub = init_hub(10);
    page_make_hub();

    switch (argc) {
    case 1:
        log_start(LogEngine);
        engine_window();

        break;
    case 3:
        if (strcmp(argv[1], "-wid") == 0) {
            log_start(LogPreview);
            wid = atoi(argv[2]);
            log_file(LogMessage, "Recieved wid %d", wid);
            preview_window(wid);
            break;
        }

        log_file(LogError, "Incorrect argument %s", argv[1]);
        return 0;
    default: 
        log_file(LogError, "Incorrect usage");
        return 0;
    }

    free_engine();
    return 1;
}


void free_engine(void) {
    shutdown(engine.socket, SHUT_RDWR);
    free_hub(engine.hub);
    log_file(LogMessage, "Engine Shutdown");
    exit(1);
}
