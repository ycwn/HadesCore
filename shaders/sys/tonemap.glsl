
#section
#ifndef _ORION_TONEMAP_GLSL_H
#define _ORION_TONEMAP_GLSL_H


#section .vert, .frag

vec3 tonemap_hdr(vec3 color)
{

	return vec3(1.0) - exp(-g_eye_exposure * color);

}


#section
#endif

