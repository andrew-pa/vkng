#version 450
#extension GL_ARB_separate_shader_objects : enable

float hash( const float n ) {
	return fract(sin(n)*43758.54554213);
}
vec2 hash2( const float n ) {
	return fract(sin(vec2(n,n+1.))*vec2(43758.5453123));
}
vec2 hash2( const vec2 n ) {
	return fract(sin(vec2( n.x*n.y, n.x+n.y))*vec2(25.1459123,312.3490423));
}
vec3 hash3( const vec2 n ) {
	return fract(sin(vec3(n.x, n.y, n+2.0))*vec3(36.5453123,43.1459123,11234.3490423));
}
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

#define saturate(v) clamp(v, 0.01, 1.01)

vec3 brdf(in vec3 base_color, in float roughness, in float metallic, in samplerCube env, in vec3 L, in vec3 Lcolor, in vec3 N, in vec3 V) {
	const vec3 H = normalize(V+L);
	const float ndh = saturate(dot(N, H)),
				vdh = saturate(dot(V, H)),
				ldh = saturate(dot(L, H)),
				ndl = saturate(dot(L, N)),
				ndv = saturate(dot(N, V));

	const float alpha2 = roughness*roughness;
	float D = distribution_ggx(ndh, alpha2), F = fresnel_schlick(vdh, metallic), G = saturate(geometry_ggx(ldh, alpha2)*geometry_ggx(vdh, alpha2));
	float spec = F*G*vdh / (ndh*ndv); //D*F*G / (4.0 * ndl * ndv);
	vec3 Kdiff = base_color;///PI;
	vec3 Kspec = spec*Lcolor;
	return Kdiff; //mix(Kdiff + Kspec, Kspec*base_color, smoothstep(0.2, 0.45, metallic));
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
	vec4 mat = texture(gbuffer[3], texCoord);
	vec3 N = normalize(norm_exist.xyz*2.-1.);
	const vec3 V = vec3(0., 0., 1.); // we're in view space already

	// calculate reflected light by integrating
	vec3 col = vec3(0.);
	for(int i = 0; i < 16; ++i) {
		vec3 L = chs(-N, hash2(hash2(texCoord) + float(i)*24.0) );
		vec3 Lc =  textureLod(env, -L, 3.0).xyz*100.0;
		col += Lc * brdf(texc, mat.x, mat.y, env, L, Lc, N, V) * saturate(dot(N, L));
	}
	

	outColor = vec4(col / 16., 1.);
}
