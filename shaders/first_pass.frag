#version 450

layout(location = 0) in vec3 fragCol;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in vec3 fragNormal;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;


layout(location = 0) out vec4 outColor;     // final output color


void main() {
    float depth = gl_FragCoord.z;
    
    //outColor = texture(textureSampler, fragUv);
    //outColor = vec4(fragCol, 1.0f);

    // Hard-coded light values
    vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0));
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    float ambientStrength = 0.1;
    
    // Normalize normal vector
    vec3 N = normalize(fragNormal);
    
    // Calculate diffuse component
    float diff = max(dot(N, lightDir), 0.0);
    
    // Calculate final color with ambient and diffuse
    vec3 ambient = ambientStrength * lightColor;
    vec3 diffuse = diff * lightColor;
    vec3 finalColor = (ambient + diffuse) * fragCol;

    outColor = vec4(finalColor, 1.0);
}