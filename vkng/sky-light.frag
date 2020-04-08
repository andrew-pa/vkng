#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "hash.h"

vec3 chs( const vec3 n, const vec2 rv2 ) {
	vec3  uu = normalize( cross( n, vec3(0.0,1.0,1.0) ) );
	vec3  vv = cross( uu, n );
	
	float ra = sqrt(rv2.y);
	float rx = ra*cos(6.2831*rv2.x); 
	float ry = ra*sin(6.2831*rv2.x);
	float rz = sqrt( 1.0-rv2.y );
	vec3  rr = vec3( rx*uu + ry*vv + rz*n );

	return normalize( rr );
}
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

#define saturate(v) clamp(v, 0.0, 1.0)

vec3 brdf(in vec3 base_color, in float roughness, in float metallic, in samplerCube env, in vec3 L, in vec3 Lcolor, in vec3 N, in vec3 V) {
	const vec3 H = normalize(V+L);
	const float ndh = saturate(dot(N, H)),
				vdh = saturate(dot(V, H)),
				ldh = saturate(dot(L, H)),
				ndl = saturate(dot(L, N)),
				ndv = saturate(dot(N, V));

	const float alpha2 = roughness*roughness;
	float D = distribution_ggx(ndh, alpha2), F = fresnel_schlick(vdh, metallic), G = saturate(geometry_ggx(ldh, alpha2)*geometry_ggx(vdh, alpha2));
	float spec = F*G*vdh / (ndh*ndv); // D*F*G / (4.0 * ndl * ndv);
	vec3 Kdiff = base_color/PI;
	vec3 Kspec = spec*Lcolor;
	return Kdiff;//mix(Kdiff + Kspec, Kspec*base_color, smoothstep(0.2, 0.45, metallic));
}
//#include <shade.h>

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D gbuffer[4];
layout(binding = 1) uniform samplerCube env;

layout(push_constant) uniform PushConstants {
	mat4 view_inv;
} pc;


vec3 importance_sample_ggx(vec2 Xi, float roughness, vec3 N) {
	float a = roughness*roughness;
	float phi = 2.0 * PI * Xi.x;
	float costheta = sqrt((1.0 - Xi.y) / (1.0+(a*a - 1) * Xi.y));
	float sintheta = sqrt(1.0 - costheta*costheta);
	vec3 H = vec3(sintheta * cos(phi), sintheta * sin(phi), costheta);
	vec3 up = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
	vec3 tx = normalize(cross(up, N));
	vec3 ty = cross(N, tx);
	return tx * H.x + ty * H.y + N * H.z;
}


void main() {
	vec4 norm_exist = texture(gbuffer[1], texCoord);
	vec3 base_color = texture(gbuffer[2], texCoord).xyz;
	if(norm_exist.w < 1.) {
		discard;
	}
	vec4 mat = texture(gbuffer[3], texCoord);
	vec3 N = normalize(norm_exist.xyz*2.-1.);
	const vec3 V = vec3(0., 0., 1.); // we're in view space already

	// calculate reflected light by integrating
	vec3 Kd = vec3(0.), Ks = vec3(0.);
	#define NUM_DIFFUSE_SAMPLES 32
	#define NUM_SPECULAR_SAMPLES 16
	#define L_B 2.0
	for(int i = 0; i < NUM_DIFFUSE_SAMPLES; ++i) {
		vec3 L = chs(N, hash23(vec3(texCoord, float(i)*6.0)));
		vec3 Lc = textureLod(env, (pc.view_inv*vec4(L,0.0)).xyz, 7.0).xyz * L_B;
		Kd += Lc * base_color/PI * saturate(dot(N, L));
	}
	Kd /= float(NUM_DIFFUSE_SAMPLES);
	//Kd = base_color/PI;
	for(int i = 0; i < NUM_SPECULAR_SAMPLES; ++i) {
		vec3 H = importance_sample_ggx(hash23(vec3(texCoord, float(i)*6.0)), mat.x*mat.x, N);
		vec3 L = 2.0 * dot(V, H) * H - V;
		float ndl = saturate(dot(N, L));
		if(ndl > 0.0) {
			float ndv = saturate(dot(N, V));
			float ndh = saturate(dot(N, H));
			float vdh = saturate(dot(V, H));
			vec3 Lc = textureLod(env, (pc.view_inv*vec4(L,0.0)).xyz, 10.0).xyz * L_B;
			float G = saturate(geometry_ggx(ndv, mat.x)*geometry_ggx(ndl, mat.x));
			float F = fresnel_schlick(vdh, .9);
			Ks += Lc * F * G * (vdh / ndh*ndv);// * F * G * vdh / (ndh * ndv);
		}
	}
	Ks /= float(NUM_SPECULAR_SAMPLES);
	
	//outColor = vec4(mix(Kd + Ks, Ks*base_color, smoothstep(0.2, 0.45, mat.y)), 1.);

	outColor = vec4(Kd+Ks, 1.);
}
