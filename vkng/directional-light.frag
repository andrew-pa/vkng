#version 450
#extension GL_ARB_separate_shader_objects : enable

#include <shade.h>

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D gbuffer[4];
layout(binding = 1) uniform samplerCube env;

layout(push_constant) uniform PushConstants {
	vec3 d;
	vec3 color;
} L;



void main() {
	vec4 norm_exist = texture(gbuffer[1], texCoord);
	vec3 texc = texture(gbuffer[2], texCoord).xyz;
	if(norm_exist.w < 1.) {
		discard;
	}
	vec3 norm = normalize(norm_exist.xyz*2.-1.);
	outColor = vec4(shade(texc, 0.5, 0.02, env, L.d, L.color, norm, vec3(0.0, 0.0, -1.0)), 1.);
}
