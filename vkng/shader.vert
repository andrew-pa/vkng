#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view_proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 texCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = ubo.view_proj * ubo.model * vec4(inPosition, 1.0);
	texCoord = inTexCoord;
}