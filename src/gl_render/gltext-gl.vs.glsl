#version 330 core

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat3 scale;

void main(){
    vec3 resize = scale * vec3(vertex.xy, 0.0) - vec3(1.0, 1.0, 0.0);

    gl_Position = vec4(resize, 1.0);
    TexCoords = vertex.zw;
};
