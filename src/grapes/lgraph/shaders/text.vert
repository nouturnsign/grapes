#version 330

uniform mat4 mvp;
uniform float font_size_px;
uniform float font_aspect_ratio;
uniform int char_offset;
uniform vec4 font_color;
uniform sampler2DArray font_texture_sampler;

in vec2 in_vert;
in float in_char; // 32 - 126 (inclusive)

out int geom_char;

void main() {
    gl_Position = vec4(in_vert, 0.0, 1.0);
    geom_char = int(in_char);
}