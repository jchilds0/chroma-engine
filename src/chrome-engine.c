/*
 * chroma-engine.c 
 */

#include "chroma-engine.h"
#include <sys/socket.h>

Engine engine;

int main(int argc, char **argv) {
    uint wid;
    start_log(-1);

    // Graphics Hub 
    engine.hub = init_hub(10);
    make_pages();

    switch (argc) {
    case 1:
        start_log(LogEngine);
        engine_window();

        break;
    case 3:
        if (strcmp(argv[1], "-wid") == 0) {
            start_log(LogPreview);
            wid = atoi(argv[2]);
            log_to_file(LogMessage, "Recieved wid %d", wid);
            preview_window(wid);
            break;
        }

        log_to_file(LogError, "Incorrect argument %s", argv[1]);
        return 0;
    default: 
        log_to_file(LogError, "Incorrect usage");
        return 0;
    }

    free_engine();
    return 1;
}


void free_engine(void) {
    shutdown(engine.socket, SHUT_RDWR);
    free_hub(engine.hub);
    log_to_file(LogMessage, "Engine Shutdown");
    exit(1);
}
