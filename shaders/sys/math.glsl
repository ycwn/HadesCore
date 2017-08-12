
#section
#ifndef _ORION_MATH_GLSL_H
#define _ORION_MATH_GLSL_H


#section .vert, .frag

float length2(vec2 u) { return dot(u, u); }
float length2(vec3 u) {	return dot(u, u); }
float length2(vec4 u) {	return dot(u, u); }

float distance2(vec2 u, vec2 v) { return length2(u - v); }
float distance2(vec3 u, vec3 v) { return length2(u - v); }
float distance2(vec4 u, vec4 v) { return length2(u - v); }

float saturate(float u) { return clamp(u, 0.0, 1.0); }
vec2  saturate(vec2  u) { return clamp(u, vec2(0.0), vec2(1.0)); }
vec3  saturate(vec3  u) { return clamp(u, vec3(0.0), vec3(1.0)); }
vec4  saturate(vec4  u) { return clamp(u, vec4(0.0), vec4(1.0)); }


vec3 orthonormalize(vec3 u, vec3 v)
{
	return normalize(u - v * dot(u, v));
}


vec3 horizon(vec3 eye, vec3 normal, float radius)
{
	vec3 OE = normalize(eye);
	vec3 OR = orthonormalize(normal, OE);

	float q = radius / length(eye);
	float p = sqrt(1.0 - q * q);

	return OE * q + OR * p;
}


#section
#endif

