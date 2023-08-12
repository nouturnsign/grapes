#version 330

uniform mat4 mvp;

in vec2 in_vert;

void main() {
    gl_Position = mvp * vec4(in_vert, 0.0, 1.0);
}