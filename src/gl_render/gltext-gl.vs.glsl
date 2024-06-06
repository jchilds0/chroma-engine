#version 330 core

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 ortho;

void main(){
    vec4 rectPos = vec4(vertex.xy, 0.0, 1.0);

    mat4 M = ortho * view * model;

    gl_Position = M * rectPos;

    TexCoords = vertex.zw;
};
