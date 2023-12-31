#version 330 core

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 ortho;

uniform vec4 origin;

void main(){
    vec4 rectPos = vec4(vertex.xy, 0.0, 1.0);

    gl_Position = ortho * view * model * rectPos;
    TexCoords = vertex.zw;
};
