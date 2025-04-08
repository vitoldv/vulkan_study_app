#version 450        // GLSL 4.5

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;

layout(binding = 0) uniform UboProjectionView {
    mat4 projection;
    mat4 view;    
} uboProjectionView;

// LEFT FOR REFERENCE ON DYNAMIC UNIFORM BUFFERS
// layout(binding = 1) uniform UboModel {
//     mat4 model;  
// } uboModel;

layout(push_constant) uniform PushModel {
    mat4 model;
} pushModel;

layout(location = 0) out vec3 fragCol;

void main() {
    gl_Position = uboProjectionView.projection * uboProjectionView.view * pushModel.model * vec4(pos, 1.0);
    fragCol = col;
}