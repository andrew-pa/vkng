#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform samplerCube sky;

void main() {
	outColor = texture(sky, texCoord);
}
