

#include <core.glsl>

#version 100
#shader  geometry::radiance
#stage   475

#render front
#depth  lequal


#section .vert, .frag

varying vec4 v_color;
varying vec2 v_tex0;

#define color_a g_frag_v0
#define color_d g_frag_v1


#section .vert

void main()
{

	v_color = color_d;
	v_tex0  = g_texcoords.st;

	gl_Position = g_mat_mvp * g_vertex;

}


#section .frag

void main()
{

	gl_FragColor = v_color;

}
