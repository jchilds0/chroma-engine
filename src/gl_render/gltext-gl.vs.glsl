#version 330 core

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 ortho;

uniform vec4 origin;

void main(){
    vec4 rectPos = vec4(vertex.x, vertex.y, -1.0, 1.0);
    vec4 modelPos = model * rectPos;
    vec4 projPos = projection * vec4(modelPos.x / modelPos.w, modelPos.y / modelPos.w, modelPos.z / modelPos.w, 1.0);
    vec4 fPos= vec4(projPos.x / projPos.w, projPos.y / projPos.w, 0.0, 1.0);

    gl_Position = ortho * view * (fPos + origin);
    TexCoords = vertex.zw;
};
