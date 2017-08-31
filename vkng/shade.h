
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

vec3 shade(in vec3 base_color, in float roughness, in float metallic, in samplerCube env, in vec3 L, in vec3 Lcolor, in vec3 N, in vec3 V) {
	const vec3 H = normalize(V+L);
	const float ndh = abs(dot(N, H)),
		  		vdh = abs(dot(V, H)),
				ldh = abs(dot(L, H)),
				ndl = abs(dot(L, N)),
				ndv = abs(dot(N, V));
	const float alpha2 = roughness*roughness;
	float spec = distribution_ggx(ndh, alpha2)*fresnel_schlick(vdh, metallic)*geometry_ggx(ldh, alpha2)*geometry_ggx(vdh, alpha2);
	spec /= 4.0 * ndl * ndv;
	vec3 Kdiff = base_color/PI;
	vec3 Kspec = spec*texture(env, L).xyz*Lcolor;
	return vec3(0.);//mix(Kdiff + Kspec, Kspec*base_color, smoothstep(0.2, 0.45, metallic));
}
