#version 450

layout(location = 0) in vec3 fragCol;
layout(location = 0) out vec4 outColor;     // final output color


void main() {
    float depth = gl_FragCoord.z;
    outColor = vec4(fragCol, 1.0);
}