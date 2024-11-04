/*
 * gl_shaders.c
 *
 * Shaders for GL Renderer
 *
 */

#include "gl_render_internal.h"

const char *glimage_vs_glsl = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;"
"layout (location = 1) in vec3 aColor;"
"layout (location = 2) in vec2 aTexCoord;"

"uniform mat4 model;"
"uniform mat4 view;"
"uniform mat4 ortho;"

"out vec3 outColor;"
"out vec2 TexCoord;"

"void main(){"
    "gl_Position = ortho * view * model * vec4(aPos, 1.0);"
    "outColor = aColor;"
    "TexCoord = aTexCoord;"
"};";

const char *glimage_fs_glsl = "#version 330 core\n"
"out vec4 FragColor;"

"in vec3 ourColor;"
"in vec2 TexCoord;"

"uniform sampler2D ourTexture;"

"void main(){"
    "vec4 texColor = texture(ourTexture, TexCoord);"
    "if (texColor.a < 0.1) {"
        "discard;"
    "}"

    "FragColor = texColor;"
"};";

const char *glrender_vs_glsl = "#version 330 core\n"
"layout(location = 0) in vec2 position;"
"layout(location = 1) in vec4 color;"
"layout(location = 2) in vec2 uv;"

"out vec4 out_color;"
"out vec2 out_uv;"

"uniform mat4 model;"
"uniform mat4 view;"
"uniform mat4 ortho;"

"void main(){"
    "out_color = color;"
    "out_uv = uv;"

    "gl_Position = ortho * view * model * vec4(position, 0.0, 1.0);"
"};";

const char *glrender_fs_glsl = "#version 330 core\n"
"in vec4 out_color;\n"
"in vec2 out_uv;\n"

"void main() {\n"
    "gl_FragColor = out_color;\n"
"}";

const char *glshape_vs_glsl = "#version 330 core\n"
"layout (location = 0) in vec3 vertex;"

"uniform mat4 model;"
"uniform mat4 view;"
"uniform mat4 ortho;"

"void main(){"
    "gl_Position = ortho * view * model * vec4(vertex, 1.0);"
"};";

const char *glshape_fs_glsl = "#version 330 core\n"
"uniform vec4 color;"
"out vec4 FragColor;"

"void main(){"
    "FragColor = color;"
"};";

const char *gltext_vs_glsl = "#version 330 core\n"
"layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
"out vec2 TexCoords;"

"uniform mat4 model;"
"uniform mat4 view;"
"uniform mat4 ortho;"

"void main(){"
    "vec4 rectPos = vec4(vertex.xy, 0.0, 1.0);"

    "mat4 M = ortho * view * model;"

    "gl_Position = M * rectPos;"

    "TexCoords = vertex.zw;"
"};";

const char *gltext_fs_glsl = "#version 330 core\n"
"in vec2 TexCoords;"
"out vec4 FragColor;"

"uniform sampler2D text;"
"uniform vec4 color;"

"void main(){"
    "vec4 sampled = vec4(1.0, 1.0, 1.0, texture2D(text, TexCoords).r);"
    "FragColor = color * sampled;"

"};";
