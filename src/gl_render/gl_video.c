/*
 * gl_video.c
 *
 * Setup and render a video described by a 
 * GeometryVideo in a GL context.
 */

#include "chroma-engine.h"
#include "geometry.h"
#include "gl_render_internal.h"
#include "log.h"
#include <libavcodec/codec.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>

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

int gl_video_load_mp4(char *filename);
unsigned char *gl_video_read_frame(AVFormatContext *format_ctx, int video_stream_idx, 
                                   AVCodecContext *codec_ctx, AVFrame *frame);

void gl_video_init_buffers(void) {
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

void gl_video_init_shaders(void) {
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

void gl_draw_video(IGeometry *geo) {
    int pos_x = geometry_get_int_attr(geo, "pos_x");
    int pos_y = geometry_get_int_attr(geo, "pos_y");
    char video_path[1024];

    memset(video_path, '\0', sizeof video_path);
    geometry_get_attr(geo, "string", video_path);

    // update data if img has changed 
    if (strcmp(path, video_path) != 0) { 
        free(data);
        strcpy(path, video_path);

        if (gl_video_load_mp4(&video_path[0]) < 0) {
            return;
        }
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


int gl_video_load_mp4(char *filename) {
    AVFormatContext *format_ctx = NULL;

    char file_path[1024];
    memset(file_path, '\0', sizeof file_path);
    memcpy(file_path, INSTALL_DIR, strlen( INSTALL_DIR ));
    memcpy(&file_path[strlen(INSTALL_DIR)], filename, strlen(filename));

    if (avformat_open_input(&format_ctx, file_path, NULL, NULL) < 0) {
        log_file(LogWarn, "GL Render", "Error opening video %s", file_path); 
        return -1;
    }

    if (avformat_find_stream_info(format_ctx, NULL) < 0) {
        log_file(LogWarn, "GL Render", "Error finding %s video stream information", file_path);
        return -1;
    }

    int video_stream_idx = -1;
    for (int i = 0; i < format_ctx->nb_streams; i++) {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }

    if (video_stream_idx == -1) {
        log_file(LogWarn, "GL Render", "Error finding %s video stream", file_path);
        avformat_close_input(&format_ctx);
        return -1;
    }

    AVCodecParameters *codec_params = format_ctx->streams[video_stream_idx]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codec_params->codec_id);
    if (!codec) {
        log_file(LogWarn, "GL Render", "Error %s unsupported codec", file_path);
        avformat_close_input(&format_ctx);
        return -1;
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        log_file(LogWarn, "GL Render", "Failed to copy codec params to context (%s)", file_path);
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return -1;
    }

    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        log_file(LogWarn, "GL Render", "Failed to open codec (%s)", file_path);
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return -1;
    }

    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        log_file(LogWarn, "GL Render", "Failed to allocate frame (%s)", file_path);
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return -1;
    }

    return 0;
}

unsigned char *gl_video_read_frame(AVFormatContext *format_ctx, int video_stream_idx, 
            AVCodecContext *codec_ctx, AVFrame *frame) {
    AVPacket packet;
    int frame_finished;
    int ret;
    unsigned char *data;

    if (av_read_frame(format_ctx, &packet) < 0) {
        return NULL;
    }

    if (packet.stream_index != video_stream_idx) {
        av_packet_unref(&packet);
        return NULL;
    }

    ret = avcodec_send_packet(codec_ctx, &packet);
    if (ret < 0) {
        log_file(LogWarn, "GL Render", "Error sending a packet for decoding");
        return NULL;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(codec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return NULL;
        } else if (ret < 0) {
            log_file(LogWarn, "GL Render", "Error during decoding video stream");
            return NULL;
        }

        // process frame
        frame->width = 1;
    }

    av_packet_unref(&packet);
    return data;
}
