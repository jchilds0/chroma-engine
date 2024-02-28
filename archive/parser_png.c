/*
 * parser_png.c 
 *
 * Load a png and extract the data into an array 
 */

#include "chroma-engine.h"
#include "log.h"
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <png.h>

#define CHUNK_SIZE    16384

typedef struct {
    unsigned int      width;
    unsigned int      height;
    unsigned char     bit_depth;
    unsigned char     color_type;
    unsigned char     comp_method;
    unsigned char     filter_method;
    unsigned char     interlace_method;
} IHDR;

typedef struct {
    unsigned char     red;
    unsigned char     green;
    unsigned char     blue;
} PLTE;

static int parser_png_read_sig(FILE *f) {
    unsigned char signature[8];
    unsigned char png_sig[] = {137, 80, 78, 71, 13, 10, 26, 10};

    fread(&signature[0], sizeof( unsigned char ), sizeof(signature), f);

    for (int i = 0; i < 8; i++) {
        if (signature[i] == png_sig[i]) {
            continue;
        }

        log_file(LogWarn, "Parser", "PNG Header incorrect %d-th byte, expected %d, recieved %d", i, png_sig[i], signature[i]);
        return -1;
    }

    return 0;
}

static unsigned int parser_png_chunk_1b(FILE *f) {
    unsigned char byte;
    fread(&byte, sizeof( byte ), 1, f);

    return byte;
}

static unsigned int parser_png_chunk_4b(FILE *f) {
    unsigned int byte;
    fread(&byte, sizeof( byte ), 1, f);

    return byte;
}

static void parser_png_read_ihdr(FILE *f, unsigned int length, IHDR *ihdr) {
    unsigned char byte;
    fseek(f, length, sizeof( byte ));
}

static void parser_png_read_plte(FILE *f, unsigned int length, PLTE *plte) {
    unsigned char byte;
    fseek(f, length, sizeof( byte ));
}

static int parser_png_read_idat(FILE *f, unsigned int length) {
    z_stream zstream;

    zstream.zalloc = Z_NULL;
    zstream.zfree = Z_NULL;
    zstream.opaque = Z_NULL;
    zstream.avail_in = 0;
    zstream.next_in = Z_NULL;

    int ret = inflateInit(&zstream);
    if (ret != Z_OK) {
        log_file(LogWarn, "Parser", "Error initialising zlib");
        return -1;
    }

    unsigned char in_buffer[CHUNK_SIZE];
    unsigned char out_buffer[CHUNK_SIZE];

    while (ret != Z_STREAM_END) {
        zstream.avail_in = fread(in_buffer, sizeof( unsigned char ), CHUNK_SIZE, f);
        if (ferror(f)) {
            log_file(LogWarn, "Parser", "Error reading PNG file");
            inflateEnd(&zstream);
            return -1;
        }

        zstream.next_in = in_buffer;

        while (zstream.avail_out == 0) {
            zstream.avail_out = CHUNK_SIZE;
            zstream.next_out = out_buffer;

            ret = inflate(&zstream, Z_NO_FLUSH);
            switch (ret) {
                case Z_NEED_DICT:
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    log_file(LogWarn, "Parser", "Error decompressing PNG data");
                    inflateEnd(&zstream);
                    return -1;
            }

            fwrite(out_buffer, 1, CHUNK_SIZE - zstream.avail_out, stdout);
        }
    }

    inflateEnd(&zstream);

    return 0;
}

static void parser_png_read_iend(FILE *f, unsigned int length, IEND *iend) {
}

// TODO: Transparency chunk parsing

unsigned char *gl_image_load_png(char *filename, int *w, int *h) {
    char file_path[1024];
    memset(file_path, '\0', sizeof file_path);
    memcpy(file_path, INSTALL_DIR, strlen( INSTALL_DIR ));
    memcpy(&file_path[strlen(INSTALL_DIR)], filename, strlen(filename));

    FILE *file = fopen(file_path, "r");

    if (parser_png_read_sig(file) < 0) {
        log_file(LogWarn, "Parser", "Incorrect Header on %s", file_path);
        return NULL;
    }

    parser_png_read_ihdr();
    parser_png_read_ihdr();
    parser_png_read_plte();
    parser_png_read_idat();
    parser_png_read_iend();

    fclose(file);
}

