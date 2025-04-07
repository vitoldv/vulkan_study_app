#version 450        // GLSL 4.5

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;

layout(binding = 0) uniform UboProjectionView {
    mat4 projection;
    mat4 view;    
} uboProjectionView;

layout(binding = 1) uniform UboModel {
    mat4 model;  
} uboModel;

layout(location = 0) out vec3 fragCol;

void main() {
    gl_Position = uboProjectionView.projection * uboProjectionView.view * uboModel.model * vec4(pos, 1.0);
    fragCol = vec3(1.0, 0.0, 0.0);
}