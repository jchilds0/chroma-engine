/*
 * Recieve graphics request over tcp and render to GtkDrawingArea
 */

#include "chroma-engine.h"
#include <sys/socket.h>

void render_page(Graphics *hub, char *buf);

void renderer(cairo_t *cr) {
    int socket_client = -1, rec;

    char buf[MAX_BUF_SIZE];
    memset(buf, '\0', sizeof buf );

    if (socket_client < 0) {
        socket_client = listen_for_client(engine.socket);
    } else {
        rec = recieve_message(socket_client, buf);

        switch (rec) {
            case CHROMA_MESSAGE:
                //render_page(engine->hub, buf);
                log_to_file(LogMessage, "Render request %s", buf); 
                break;
            case CHROMA_TIMEOUT:
                break;
            case CHROMA_CLOSE_SOCKET:
                shutdown(socket_client, SHUT_RDWR);
                socket_client = -1;
        }
    }
}


void render_page(Graphics *hub, char *buf) {
    int v_m, v_n, length, action, temp_num;
    sscanf(buf, "ver%d,%d#len%d#action%d#temp%d#", 
           &v_m, &v_n, &length, &action, &temp_num);

    printf("Recieved: %s\n", buf);
    printf("Parsed - ver%d.%d, len %d, action %d, temp %d\n", v_m, v_n, length, action, temp_num);

    if (!(v_m == 1 && v_n == 0)) {
        printf("Incorrect version v%d.%d, expected v1.0\n", v_m, v_n);
        return;
    }

    if (action == ANIMATE_ON) {
        animate_off_page(hub, hub->current_page);
    }

    // skip header 
    int i = 0, num_hash = 0;
    while (num_hash < 4) {
        if (buf[i] == '#') {
            num_hash++;
        }
        i++;
    }

    char attr[MAX_BUF_SIZE];
    char value[MAX_BUF_SIZE];

    while (buf[i] != END_OF_MESSAGE) {
        if (i >= MAX_BUF_SIZE) {
            // handle error
            printf("Error: missing end of message tag\n");
            return;
        }

        num_hash = 0;
        memset(attr, '\0', sizeof attr);
        memset(value, '\0', sizeof value);
        for (int j = i; buf[j] != '\0'; j++) {
            if (buf[j] != '#')
                continue;

            if (num_hash == 0) {
                memcpy(attr, &buf[i], j - i);
            } else if (num_hash == 1) {
                memcpy(value, &buf[i], j - i);
                set_page_attr(hub->pages[temp_num], attr, value);
                printf("Found: attr %s, value %s\n", attr, value);

                i = j + 1;
                break;
            }

            i = j + 1;
            num_hash++;
        }

        if (num_hash != 1) {
            // handle error
            printf("Error: Unknown attr");
            return;
        }

    }

    switch (action) {
        case ANIMATE_ON:
            animate_on_page(hub, temp_num);
            break;
        case CONTINUE:
            continue_page(hub, temp_num);
            break;
        case ANIMATE_OFF:
            animate_off_page(hub, temp_num);
            break;
    }
}

