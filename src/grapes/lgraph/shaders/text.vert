#version 330

in vec2 in_vert;
in float in_char; // 32 - 126 (inclusive)

out int geom_char;

void main() {
    gl_Position = vec4(in_vert, 0.0, 1.0);
    geom_char = int(in_char);
}