#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D text;
uniform vec4 color;

void main(){
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture2D(text, TexCoords).r);

    FragColor = color * sampled;
};
