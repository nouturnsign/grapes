#version 330

uniform vec4 font_color;
uniform sampler2DArray font_texture_sampler;

flat in int frag_index;
in vec2 uv;

out vec4 color;

void main() {
    color = texture(font_texture_sampler, vec3(uv, frag_index)).rgba * font_color;
}