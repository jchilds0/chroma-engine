/*
 * gl_image.c
 *
 * Setup and render an image described by a 
 * GeometryImage in a GL context.
 *
 */

#include "chroma-engine.h"
#include "geometry.h"
#include "gl_render_internal.h"
#include "log.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GLuint vao;
static GLuint vbo;
static GLuint ebo;
static GLuint texture;
static GLuint program;

static GLuint indices[] = {
    0, 1, 3, // first triangle 
    1, 2, 3, // second triangle
};

unsigned char *gl_image_load_png(char *filename, int *w, int *h);

void gl_image_init_buffers(void) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    // bind buffer arrays 
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof( float ) * 4 * 8, NULL, GL_STATIC_DRAW);

    // bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);

    // position attribute 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), (void *)(0));
    glEnableVertexAttribArray(0);

    // color attribute 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), (void *)(3 * sizeof( float )));
    glEnableVertexAttribArray(1);

    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), (void *)(6 * sizeof( float )));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void gl_image_init_shaders(void) {
    char *vertexSource = gl_renderer_get_shader_file(INSTALL_DIR SHADER_PATH "glimage-gl.vs.glsl");
    char *fragmentSource = gl_renderer_get_shader_file(INSTALL_DIR SHADER_PATH "glimage-gl.fs.glsl");

    GLuint vertex = gl_renderer_create_shader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = gl_renderer_create_shader(GL_FRAGMENT_SHADER, fragmentSource);

    program = gl_renderer_create_program(vertex, fragment);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    free(vertexSource);
    free(fragmentSource);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

static char path[1024] = "";
static unsigned char *data;
static int w, h;

void gl_draw_image(IGeometry *geo) {
    int pos_x = geometry_get_int_attr(geo, "pos_x");
    int pos_y = geometry_get_int_attr(geo, "pos_y");
    char img_path[1024];

    memset(img_path, '\0', sizeof img_path);
    geometry_get_attr(geo, "string", img_path);

    // update data if img has changed 
    if (strcmp(path, img_path) != 0) { 
        free(data);
        strcpy(path, img_path);

        data = gl_image_load_png(&img_path[0], &w, &h);
    }

    if (data == NULL) {
        return;
    }

    char buf[100];
    memset(buf, '\0', sizeof( buf ));
    geometry_get_attr(geo, "scale", buf);
    float scale = atof(buf);

    GLfloat vertices[] = {
        // positions                                  // colors           // texture coords
        pos_x + w * scale, pos_y + h * scale, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
        pos_x + w * scale, pos_y,             0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        pos_x,             pos_y,             0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom right
        pos_x,             pos_y + h * scale, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f, // bottom right
    };

    gl_renderer_set_scale(program);

    glUseProgram(program);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

static int gl_image_read_png(char *filename, int *w, int *h, 
            png_byte *color_type, png_byte *bit_depth, png_bytep **row_pointers) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        log_file(LogWarn, "GL Render", "Error opening %s", filename);
        return -1;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        log_file(LogWarn, "GL Render", "Error creating png read structure");
        fclose(fp);
        return -1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        log_file(LogWarn, "GL Render", "Error creating png info structure");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return -1;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        //log_file(LogWarn, "GL Render", "Error during png reading");
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

unsigned char *gl_image_load_png(char *filename, int *w, int *h) {
    char file_path[1024];
    memset(file_path, '\0', sizeof file_path);
    memcpy(file_path, INSTALL_DIR, strlen( INSTALL_DIR ));
    memcpy(&file_path[strlen(INSTALL_DIR)], filename, strlen(filename));

    png_byte color_type, bit_depth;
    png_bytep *row_pointers = NULL;

    if (gl_image_read_png(file_path, w, h, &color_type, &bit_depth, &row_pointers) < 0) {
        //log_file(LogWarn, "GL Render", "Error reading png");
        return NULL;
    }

    //log_file(LogMessage, "GL Render", "Color Type %d, Bit Depth %d", color_type, bit_depth);

    unsigned char *data = NEW_ARRAY((*w) * (*h) * 4, unsigned char);

    for (int y = 0; y < *h; y++) {
        memcpy(&data[4 * (*w) * y], row_pointers[*h - y - 1], 4 * (*w) * sizeof( unsigned char ));
    }

    free_row_pointers(*h, row_pointers);

    return data;
}

