

#include <core.glsl>
#include <math.glsl>

#version 100
#shader  skylight::ambient
#stage   525

#blend add, one, one
#faces back
#depth lequal
#write depth:off


#section .vert, .frag

#define Z g_frag_v0.rgb
#define A g_frag_v1.rgb
#define B g_frag_v2.rgb
#define C g_frag_v3.rgb
#define D g_frag_v4.rgb
#define E g_frag_v5.rgb

varying vec4 v_color;

const float farplane = 7500.0;

const vec3 XYZ_r = vec3(+3.240479, -1.537150, -0.498535);
const vec3 XYZ_g = vec3(-0.969256, +1.875992, +0.041556);
const vec3 XYZ_b = vec3(+0.055648, -0.204043, +1.057311);


#section .vert

void main()
{

	vec4 position = g_mat_mvp * vec4(g_vertex.xyz * farplane, 0.0);

	float cos_theta  = abs(g_vertex.y) + 1.0e-3;
	float cos_gamma2 = g_vertex.z * g_vertex.z;
	float gamma      = g_extra.x;

	vec3 Yxy = Z * (vec3(1.0) + A * exp(B / cos_theta)) * (vec3(1.0) + C * exp(D * gamma) + E * cos_gamma2);
	vec3 XYZ = vec3(
			Yxy.r * Yxy.g / Yxy.b,
			Yxy.r,
			Yxy.r * (1.0 - Yxy.g - Yxy.b) / Yxy.b);

	v_color.r = dot(XYZ_r, XYZ);
	v_color.g = dot(XYZ_g, XYZ);
	v_color.b = dot(XYZ_b, XYZ);
	v_color.a = 1.0;

	gl_Position = position.xyzz;

}


#section .frag

void main()
{

	gl_FragColor = v_color + vec4(0.0, 0.0, 0.125, 0.0);//vec4(0.0, 1.0, 0.0, 1.0);

}

