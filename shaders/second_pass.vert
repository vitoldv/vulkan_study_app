#version 450        // GLSL 4.5

// triangle in NDC space that covers the whole screen
vec2 positions[3] = vec2[] (
    vec2(3.0, -1.0),
    vec2(-1.0, -1.0),
    vec2(-1.0, 3.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}