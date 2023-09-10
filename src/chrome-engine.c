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
void render_pixels(Color *, int, int);
RenderObject *pixel_str_to_render_object(char *);
void write_pixel_str_to_pixels(Color *, char *);

#define MAX_BUF_SIZE      1000

int main(int argc, char **argv) {
    const int screen_width = 800;
    const int screen_height = 450;

    InitWindow(screen_width, screen_height, "raylib [core] example - basic window");
    SetTargetFPS(CHROMA_FRAMERATE);

    int socket_engine = start_tcp_server("127.0.0.1", 6100);
    int socket_client = -1, rec;
    Color pixels[screen_width * screen_height];
    RenderObject *pixel;

    char buf[MAX_BUF_SIZE];
    memset(buf, '\0', sizeof buf );

    for (int i = 0; i < screen_width * screen_height; i++) {
        pixels[i] = BLACK;
    }
    
    while (!WindowShouldClose()) {
        BeginDrawing();

        if (socket_client < 0) {
            socket_client = listen_for_client(socket_engine);
        } else {
            while ((rec = recieve_message(socket_client, buf)) >= 0) {
                pixel = pixel_str_to_render_object(buf);
                pixels[pixel->pos_y * screen_width + pixel->pos_x] = pixel->color;
                memset(buf, '\0', sizeof buf );
            }

            if (rec == CHROMA_CLOSE_SOCKET) {
                shutdown(socket_client, SHUT_RDWR);
                socket_client = -1;
            } 
        }
        render_pixels(pixels, screen_width, screen_height);

        EndDrawing();
    }

    shutdown(socket_engine, SHUT_RDWR);

    CloseWindow();
    return 0;
}

void render_text(char *buf) {
    DrawText(buf, 190, 200, 20, RAYWHITE);
}

void render_pixels(Color *pixels, int width, int height) {
    for (int i = 0; i < width * height; i++) {
        DrawPixel(i % width, i / width, pixels[i]);
    }
}

RenderObject *pixel_str_to_render_object(char *buf) {
    int tuple_index = 0, char_index = 0;
    char temp_buf[20];
    int array[6];
    RenderObject *pixel = (RenderObject *) malloc( sizeof(RenderObject) ); 

    for (int i = 1; i < strlen(buf); i++) {
        switch (buf[i]) {
            case '(':
                char_index = 0;
                break;
            case ')':
                temp_buf[char_index] = '\0';
                array[tuple_index] = atoi(temp_buf);
                tuple_index = 0;

                *pixel = (RenderObject) {array[0], array[1], (Color) {array[2], array[3], array[4], array[5]}};
                return pixel;
            case ',':
                if (pixel) {
                    temp_buf[char_index] = '\0';
                    array[tuple_index++] = atoi(temp_buf);
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
