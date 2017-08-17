#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
} ubo;

layout(push_constant) uniform PushConstants {
	mat4 view_proj;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 worldCoord;
layout(location = 2) out vec3 worldNormal;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	vec4 wc = ubo.model * vec4(inPosition, 1.0);
    gl_Position = pc.view_proj * wc;
	worldCoord = wc.xyz;
	worldNormal = (ubo.model * vec4(inNormal, 0.0)).xyz;
	texCoord = inTexCoord;
}
