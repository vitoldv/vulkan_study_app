#version 450

layout(location = 0) in vec3 fragCol;
layout(location = 1) in vec2 fragUv;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;


layout(location = 0) out vec4 outColor;     // final output color


void main() {
    float depth = gl_FragCoord.z;
    outColor = texture(textureSampler, fragUv);
    //outColor = vec4(fragUv.x, fragUv.y, 0.0f, 1.0f);
}