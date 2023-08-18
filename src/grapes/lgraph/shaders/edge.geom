#version 330
layout (lines) in;
layout (triangle_strip, max_vertices = 10) out;

uniform mat4 mvp;
uniform float node_radius;
uniform float edge_segment_width;
uniform float edge_arrowhead_width;
uniform float edge_arrowhead_height;
uniform int arrow_style;

void draw_arrowhead(vec2 midpoint, float width, float height, float costheta, float sintheta) {
    gl_Position = mvp * vec4(midpoint.x + height * costheta, midpoint.y + height * sintheta, 0.0, 1.0);
    EmitVertex();
    gl_Position = mvp * vec4(midpoint.x + width / 2.0 * -sintheta, midpoint.y + width / 2.0 * costheta, 0.0, 1.0);
    EmitVertex();
    gl_Position = mvp * vec4(midpoint.x - width / 2.0 * -sintheta, midpoint.y - width / 2.0 * costheta, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();
}

void draw_segment(vec2 src_midpoint, vec2 dst_midpoint, float width, float costheta, float sintheta) {
    gl_Position = mvp * vec4(src_midpoint.x + width / 2.0 * sintheta, src_midpoint.y - width / 2.0 * costheta, 0.0, 1.0);
    EmitVertex();
    gl_Position = mvp * vec4(src_midpoint.x - width / 2.0 * sintheta, src_midpoint.y + width / 2.0 * costheta, 0.0, 1.0);
    EmitVertex();
    gl_Position = mvp * vec4(dst_midpoint.x + width / 2.0 * sintheta, dst_midpoint.y - width / 2.0 * costheta, 0.0, 1.0);
    EmitVertex();
    gl_Position = mvp * vec4(dst_midpoint.x - width / 2.0 * sintheta, dst_midpoint.y + width / 2.0 * costheta, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();
}

void main() {
    // read from input layout
    vec2 src_position = vec2(gl_in[0].gl_Position.x, gl_in[0].gl_Position.y);
    vec2 dst_position = vec2(gl_in[1].gl_Position.x, gl_in[1].gl_Position.y);

    // compute cos(theta) and sin(theta)
    float theta = atan(dst_position.y - src_position.y, dst_position.x - src_position.x);
    float costheta = cos(theta);
    float sintheta = sin(theta);

    // adjust for node_radius
    vec2 src_midpoint = src_position + vec2(node_radius * costheta, node_radius * sintheta);
    vec2 dst_midpoint = dst_position - vec2(node_radius * costheta, node_radius * sintheta);

    // draw arrowheads
    vec2 disp = vec2(edge_arrowhead_height * costheta, edge_arrowhead_height * sintheta);
    if (arrow_style == 2) {
        src_midpoint = src_midpoint + disp;
        draw_arrowhead(src_midpoint, edge_arrowhead_width, edge_arrowhead_height, -costheta, -sintheta);
    }
    if (arrow_style == 1 || arrow_style == 2) {
        dst_midpoint = dst_midpoint - disp;
        draw_arrowhead(dst_midpoint, edge_arrowhead_width, edge_arrowhead_height, costheta, sintheta);
    }

    // draw segment
    draw_segment(src_midpoint, dst_midpoint, edge_segment_width, costheta, sintheta);
}