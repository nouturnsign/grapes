#version 330
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 mvp;
uniform float font_size_px;
uniform float font_aspect_ratio;
uniform int char_offset;

in int geom_char[1];

flat out int frag_index;
out vec2 uv;

void main() {
    vec2 point = gl_in[0].gl_Position.xy;
    float box_height = font_size_px;
    float box_width = box_height * font_aspect_ratio;

    frag_index = geom_char[0] - char_offset;

    gl_Position = mvp * vec4(point.x - box_width / 2.0, point.y + box_height / 2.0, 0.0, 1.0);
    uv = vec2(0.0, 0.0);
    EmitVertex();

    gl_Position = mvp * vec4(point.x + box_width / 2.0, point.y + box_height / 2.0, 0.0, 1.0);
    uv = vec2(1.0, 0.0);
    EmitVertex();
        
    gl_Position = mvp * vec4(point.x - box_width / 2.0, point.y - box_height / 2.0, 0.0, 1.0);
    uv = vec2(0.0, 1.0);
    EmitVertex();
        
    gl_Position = mvp * vec4(point.x + box_width / 2.0, point.y - box_height / 2.0, 0.0, 1.0);
    uv = vec2(1.0, 1.0);
    EmitVertex();
}