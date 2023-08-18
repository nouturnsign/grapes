#version 330

uniform vec4 fill_color;

out vec4 color;

void main() {
    color = vec4(fill_color);
}