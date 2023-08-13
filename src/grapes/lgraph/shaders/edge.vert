#version 330

uniform mat4 mvp;
uniform float node_radius;
uniform float edge_segment_width;
uniform float edge_arrowhead_width;
uniform float edge_arrowhead_height;
uniform int arrow_style; // 0 for none, 1 for directed, 2 for undirected

in vec2 in_vert;

void main() {
    gl_Position = vec4(in_vert, 0.0, 1.0);
}