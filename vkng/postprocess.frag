#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D final;

void main() {
	outColor = pow(texture(final, texCoord), vec4(1.0 / 2.2));
}
