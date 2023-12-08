#version 330 core

layout (location = 0) in vec3 vertex;

uniform mat2 scale;

void main(){
   gl_Position = vec4(scale * vertex.xy - vec2(1.0, 1.0), 0.0, 1.0);
};

