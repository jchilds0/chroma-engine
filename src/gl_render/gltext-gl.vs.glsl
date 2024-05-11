#version 330 core

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;
out float Mask;

uniform mat4 model;
uniform mat4 view;
uniform mat4 ortho;

uniform vec4 bound;

void main(){
    vec4 rectPos = vec4(vertex.xy, 0.0, 1.0);

    gl_Position = ortho * view * model * rectPos;

    bool x_in = bound.x <= vertex.x && vertex.x <= bound.y;
    bool y_in = bound.z <= vertex.y && vertex.y <= bound.w;

    Mask = (x_in && y_in) ? 1.0 : 0.0;

    TexCoords = vertex.zw;
};
