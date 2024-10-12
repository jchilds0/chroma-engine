#version 330

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;

out vec4 out_color;
out vec2 out_uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 ortho;

void main(){
    out_color = color;
    out_uv = uv;

    gl_Position = ortho * view * model * vec4(position, 0.0, 1.0);
};

