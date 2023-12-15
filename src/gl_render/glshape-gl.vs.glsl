#version 330 core

layout (location = 0) in vec3 vertex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 ortho;

void main(){
    gl_Position = ortho * view * model * vec4(vertex, 1.0);
};

