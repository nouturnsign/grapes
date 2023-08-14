#version 330
layout (points) in;
layout (triangle_strip, max_vertices = 90) out;

uniform mat4 mvp;
uniform float node_radius;

void draw_circle(float radius) {
    gl_Position = gl_in[0].gl_Position + mvp * vec4(0.0, -radius, 0.0, 0.0);
    EmitVertex();
    float x, y, theta;
    for (int i = 1; i < 45; ++i) {
        theta = (i / 45.0 - 0.5) * 3.1415926535897932384626433832795;
        x = radius * cos(theta);
        y = radius * sin(theta);
        gl_Position = gl_in[0].gl_Position + mvp * vec4(x, y, 0, 0);
        EmitVertex();
        gl_Position = gl_in[0].gl_Position + mvp * vec4(-x, y, 0, 0);
        EmitVertex();
    }
    gl_Position = gl_in[0].gl_Position + mvp * vec4(0.0, radius, 0.0, 0.0);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    draw_circle(node_radius);
}