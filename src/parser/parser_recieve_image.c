/*
 * parser_recieve_image.c
 */

#include "chroma-engine.h"
#include "geometry.h"
#include "log.h"
#include "parser/parser_internal.h"
#include <png.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

static int socket_client = 0;
static int buf_ptr = 0;
static char buf[PARSE_BUF_SIZE];

void free_row_pointers(int, png_bytep *);
int parser_read_image(int *, int *, png_byte *, png_byte *, png_bytep **);
void parser_read_png_data(png_structp png_ptr, png_bytep data, size_t length);

// ver=%d,%d#
ServerResponse parser_recieve_image(int hub_socket, GeometryImage *img) {
    char msg[1024];

    if (img->cur_image_id == img->image_id) {
        return SERVER_MESSAGE;
    }

    img->cur_image_id = img->image_id;
    socket_client = hub_socket;
    memset(msg, '\0', sizeof msg);
    sprintf(msg, "ver 0 1 img %d;", img->image_id);

    if (send(hub_socket, msg, sizeof( msg ), 0) < 0) {
        log_file(LogError, "Parser", "Error requesting image %d from hub", img->image_id); 
    }

    png_byte color_type, bit_depth;
    png_bytep *row_pointers = NULL;

    if (parser_read_image(&img->w, &img->h, &color_type, &bit_depth, &row_pointers) < 0) {
        //log_file(LogWarn, "GL Render", "Error reading png");
        return SERVER_TIMEOUT;
    }

    //log_file(LogMessage, "GL Render", "Color Type %d, Bit Depth %d", color_type, bit_depth);

    img->data = NEW_ARRAY(img->w * img->h * 4, unsigned char);

    for (int y = 0; y < img->h; y++) {
        memcpy(&img->data[4 * img->w * y], row_pointers[img->h - y - 1], 4 * img->w * sizeof( unsigned char ));
    }

    free_row_pointers(img->h, row_pointers);

    return SERVER_MESSAGE;
}

int parser_read_image(int *w, int *h, png_byte *color_type, 
                      png_byte *bit_depth, png_bytep **row_pointers) {
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        log_file(LogWarn, "GL Render", "Error creating png read structure");
        return -1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        log_file(LogWarn, "GL Render", "Error creating png info structure");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return -1;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        //log_file(LogWarn, "GL Render", "Error during png reading");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return -1;
    }

    png_set_read_fn(png_ptr, NULL, parser_read_png_data);
    png_read_info(png_ptr, info_ptr);

    *w = png_get_image_width(png_ptr, info_ptr);
    *h = png_get_image_height(png_ptr, info_ptr);
    *color_type = png_get_color_type(png_ptr, info_ptr);
    *bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    if (*color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }

    if (*color_type == PNG_COLOR_TYPE_GRAY && *bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }

    if (*bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }

    if (*color_type == PNG_COLOR_TYPE_GRAY || *color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr);
    }

    png_read_update_info(png_ptr, info_ptr);
    *row_pointers = NEW_ARRAY(*h, png_bytep);

    for (int y = 0; y < *h; y++) {
        (*row_pointers)[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));
    }

    png_read_image(png_ptr, *row_pointers);
    png_read_end(png_ptr, NULL);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return 0;
}

void parser_read_png_data(png_structp png_ptr, png_bytep data, size_t length) {
    if (png_ptr == NULL) {
        return;
    }

    for (int i = 0; i < length; i++) {
        data[i] = parser_get_char(socket_client, &buf_ptr, buf);
        log_file(LogMessage, "Parser", "%d %c", data[i], data[i]); 
        // if (data[i] == 0) {
        //     png_error(png_ptr, "Read Error");
        // }
    }
}

void free_row_pointers(int h, png_bytep *row_pointers) {
    for (int y = 0; y < h; y++) {
        free(row_pointers[y]);
    }

    free(row_pointers);
}
