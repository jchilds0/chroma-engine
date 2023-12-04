/*
 * chroma-engine.c 
 */

#include "chroma-engine.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <malloc.h>

void render_page(Graphics *, char *);

int main(int argc, char **argv) {
    const int screen_width = 1920;
    const int screen_height = 1080;
    int wid;

    switch (argc) {
    case 3:
        if (strcmp(argv[1], "-wid") == 0) {
                wid = atoi(argv[2]);
                break;
        }
    default: 
        printf("Usage: chroma-engine -wid [wid]\n");
        return 0;
    }

    return 1;

    // InitWindow(screen_width, screen_height, "raylib [core] example - basic window");
    // SetTargetFPS(CHROMA_FRAMERATE);
    
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_CONTAINER(window), "child window");

    GtkWidget *child_label = gtk_label_new("Hello from child window");
    gtk_container_add(GTK_CONTAINER(window), child_label);
    gtk_widget_show_all(window);
}

void tempfunc(void);

void tempfunc(void) {
    int socket_engine = start_tcp_server("127.0.0.1", 6100);
    int socket_client = -1, rec;

    char buf[MAX_BUF_SIZE];
    memset(buf, '\0', sizeof buf );

    // Graphics Hub 
    Graphics *hub = init_hub(10);
    Page *page = init_page(1);
    set_rect(page, 0, 50, 50, 300, 50);
    add_graphic(hub, page);

    page = init_page(1);
    set_rect(page, 0, 50, 780, 500, 200);
    add_graphic(hub, page);

    page = init_page(1);
    set_rect(page, 0, 100, 100, 300, 200);
    add_graphic(hub, page);


    while (true){//!WindowShouldClose()) {
        // BeginDrawing();
        
        if (socket_client < 0) {
            socket_client = listen_for_client(socket_engine);
        } else {
            rec = recieve_message(socket_client, buf);

            switch (rec) {
                case CHROMA_MESSAGE:
                    render_page(hub, buf);
                    break;
                case CHROMA_TIMEOUT:
                    break;
                case CHROMA_CLOSE_SOCKET:
                    shutdown(socket_client, SHUT_RDWR);
                    socket_client = -1;
            }
        }

        //EndDrawing();
    }

    shutdown(socket_engine, SHUT_RDWR);
    free_hub(hub);

    //CloseWindow();
    //return 0;
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

