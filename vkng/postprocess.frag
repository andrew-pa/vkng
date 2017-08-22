#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D gbuffer[4];

void main() {
	vec4 norm_exist = texture(gbuffer[1], texCoord);
	if(norm_exist.w < 1.) discard;
	vec3 norm = normalize(norm_exist.xyz*2.-1.);
	vec3 texc = texture(gbuffer[2], texCoord).xyz;
	outColor = vec4(texc * max(0., dot(norm, vec3(0., 1., 0.))), 1.);
}
