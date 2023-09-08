/*
 * chroma-engine.c 
 */

#include "chroma-engine.h"
#include <raylib.h>
#include <string.h>
#include <sys/socket.h>
#include <malloc.h>

int main(int argc, char **argv) {
    const int screen_width = 800;
    const int screen_height = 450;

    InitWindow(screen_width, screen_height, "raylib [core] example - basic window");
    SetTargetFPS(FRAMERATE);

    int socket_engine = start_tcp_server("127.0.0.1", 6100);
    int socket_client = -1, rec;
    char msg[2000], buf[2000];
    sprintf(buf, "Waiting for connection\n")

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLACK);

        if (socket_client < 0) {
            socket_client = listen_for_client(socket_engine);
        } else {
            rec = recieve_message(socket_client, msg);

            if (rec == 1) {
                strcpy(buf, msg);
            } else if (rec == CLOSE_SOCKET) {
                shutdown(socket_client, SHUT_RDWR);
                socket_client = -1;
            }
        }

        DrawText(buf, 190, 200, 20, RAYWHITE);

        EndDrawing();
    }

    shutdown(socket_engine, SHUT_RDWR);

    CloseWindow();
    return 0;
}
