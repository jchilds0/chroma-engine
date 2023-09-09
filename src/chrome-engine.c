/*
 * chroma-engine.c 
 */

#include "chroma-engine.h"
#include <raylib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <malloc.h>

void render_text(char *);
void render_pixels(char *);

#define MAX_BUF_SIZE      1000

int main(int argc, char **argv) {
    const int screen_width = 800;
    const int screen_height = 450;

    InitWindow(screen_width, screen_height, "raylib [core] example - basic window");
    SetTargetFPS(CHROMA_FRAMERATE);

    int socket_engine = start_tcp_server("127.0.0.1", 6100);
    int socket_client = -1, rec;
    char buf[MAX_BUF_SIZE];
    memset(buf, '\0', sizeof buf );
    
    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLACK);

        if (socket_client < 0) {
            socket_client = listen_for_client(socket_engine);
        } else {
            while ((rec = recieve_message(socket_client, buf)) != CHROMA_CLOSE_SOCKET) {
                render_pixels(buf);
            }

            if (rec == CHROMA_CLOSE_SOCKET) {
                shutdown(socket_client, SHUT_RDWR);
                socket_client = -1;
            }
        }

        EndDrawing();
    }

    shutdown(socket_engine, SHUT_RDWR);

    CloseWindow();
    return 0;
}

void render_text(char *buf) {
    DrawText(buf, 190, 200, 20, RAYWHITE);
}

void render_pixels(char *buf) {
    int tuple_index = 0, char_index = 0;
    char temp_buf[20];
    bool pixel;
    int object[6];

    for (int i = 1; i < strlen(buf); i++) {
        switch (buf[i]) {
            case '(':
                char_index = 0;
                pixel = true;
                break;
            case ')':
                temp_buf[char_index] = '\0';
                object[tuple_index] = atoi(temp_buf);
                pixel = false;
                tuple_index = 0;

                DrawPixel(object[0], object[1], (Color) {object[2], object[3], object[4], object[5]});
                //printf("[%d, %d, %d, %d, %d, %d]\n", object[0], object[1], object[2], object[3], object[4], object[5]);
                break;
            case ',':
                if (pixel) {
                    temp_buf[char_index] = '\0';
                    object[tuple_index++] = atoi(temp_buf);
                    char_index = 0;
                }
                break;
            default:
                if (pixel) {
                    temp_buf[char_index++] = buf[i];
                }
        }
    }
}
