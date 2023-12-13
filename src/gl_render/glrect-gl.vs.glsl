#version 330 core

layout (location = 0) in vec3 vertex;

uniform mat3 scale;

void main(){
    gl_Position = vec4(scale * vertex.xyz - vec3(1.0, 1.0, 0.0), 1.0);
};

