/*
 * parser_png.c 
 *
 * Load a png and extract the data into an array 
 */

#include "chroma-engine.h"
#include "log.h"
#include <png.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parser_png_read_file(char *filename, int *w, int *h, 
            png_byte *color_type, png_byte *bit_depth, png_bytep **row_pointers) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        log_file(LogWarn, "Parser", "Error opening %s", filename);
        return -1;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        log_file(LogWarn, "Parser", "Error creating png read structure");
        fclose(fp);
        return -1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        log_file(LogWarn, "Parser", "Error creating png info structure");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return -1;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        log_file(LogWarn, "Parser", "Error during png reading");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return -1;
    }

    png_init_io(png_ptr, fp);
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
    fclose(fp);
    return 0;
}

static void free_row_pointers(int h, png_bytep *row_pointers) {
    for (int y = 0; y < h; y++) {
        free(row_pointers[y]);
    }

    free(row_pointers);
}

unsigned char *parser_load_png(char *filename, int *w, int *h) {
    char file_path[1024];
    memset(file_path, '\0', sizeof file_path);
    memcpy(file_path, INSTALL_DIR, strlen( INSTALL_DIR ));
    memcpy(&file_path[strlen(INSTALL_DIR)], filename, strlen(filename));

    png_byte color_type, bit_depth;
    png_bytep *row_pointers = NULL;

    if (parser_png_read_file(file_path, w, h, &color_type, &bit_depth, &row_pointers) < 0) {
        log_file(LogWarn, "Parser", "Error reading png");
        return NULL;
    }

    log_file(LogMessage, "Parser", "Color Type %d, Bit Depth %d", color_type, bit_depth);

    unsigned char *data = NEW_ARRAY((*w) * (*h) * 4, unsigned char);

    for (int y = 0; y < *h; y++) {
        memcpy(&data[4 * (*w) * y], row_pointers[*h - y - 1], 4 * (*w) * sizeof( unsigned char ));
    }

    free_row_pointers(*h, row_pointers);

    return data;
}

