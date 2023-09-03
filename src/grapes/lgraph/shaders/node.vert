#version 330

uniform mat4 mvp;

in vec2 in_vert;

void main() {
    vec4 position = mvp * vec4(in_vert, 0.0, 1.0);
    gl_Position = position;
}