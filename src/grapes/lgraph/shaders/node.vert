#version 330

uniform vec2 offsets[512];
uniform mat4 mvp;

in vec2 in_vert;

void main() {
    vec2 offset = offsets[gl_InstanceID];
    gl_Position = mvp * vec4(in_vert + offset, 0.0, 1.0);
}