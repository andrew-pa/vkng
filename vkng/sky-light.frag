#version 450
#extension GL_ARB_separate_shader_objects : enable

#define PI 3.14159

float fresnel_schlick(in float vdh, in float f0) {
	return f0 + (1.0 - f0) * pow(1.0 - vdh, 5.0);
}

float geometry_ggx(in float idh, in float alpha2) {
	const float idh2 = idh*idh;
	return (2.0*idh) / (idh + sqrt(idh2 + alpha2*(1.0 - idh2)));
}

float distribution_ggx(in float ndh, in float alpha2) {
	return alpha2 / (PI * (1.0 + ndh*ndh*(alpha2 - 1.0))*2.0);
}

#define saturate(v) clamp(v, 0.01, 1.01)

vec3 shade(in vec3 base_color, in float roughness, in float metallic, in samplerCube env, in vec3 L, in vec3 Lcolor, in vec3 N, in vec3 V) {
	const vec3 H = normalize(V+L);
	const float ndh = saturate(dot(N, H)),
				vdh = saturate(dot(V, H)),
				ldh = saturate(dot(L, H)),
				ndl = saturate(dot(L, N)),
				ndv = saturate(dot(N, V));

	const float alpha2 = roughness*roughness;
	float D = distribution_ggx(ndh, alpha2), F = fresnel_schlick(vdh, metallic), G = geometry_ggx(ldh, alpha2)*geometry_ggx(vdh, alpha2);
	float spec = saturate(D*F*G / (4.0 * ndl * ndv));
	vec3 Kdiff = base_color/PI;
	vec3 Kspec = spec*Lcolor;
	return mix(Kdiff + Kspec, Kspec*base_color, smoothstep(0.2, 0.45, metallic));
}
//#include <shade.h>

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D gbuffer[4];
layout(binding = 1) uniform samplerCube env;


void main() {
	vec4 norm_exist = texture(gbuffer[1], texCoord);
	vec3 texc = texture(gbuffer[2], texCoord).xyz;
	if(norm_exist.w < 1.) {
		discard;
	}
	vec3 N = normalize(norm_exist.xyz*2.-1.);
	const vec3 V = vec3(0., 0., 1.); // we're in view space already

	// calculate reflected light by integrating
	vec3 col = vec3(0.);
	for(int i = 0; i < 20; ++i) {
		const vec3 L = vec3(0., 1., 0.);
		col += shade(texc, 0.9, 0.02, env, L, vec3(1.), N, V);
	}

	outColor = vec4(col / 20., 1.);
}
