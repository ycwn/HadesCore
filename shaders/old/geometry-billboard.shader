

#include <core.glsl>

#version 100
#shader  geometry::billboard
#stage   450

#blend add, one, one
#faces front
#write depth:off


#section .vert, .frag

varying vec2 v_tex0;

#define ambient  g_frag_v0
#define diffuse  g_frag_v1


#section .vert

void main()
{

	mat4 inverse = mat4(
		g_mat_view[0].x, g_mat_view[1].x, g_mat_view[2].x, 0.0,
		g_mat_view[0].y, g_mat_view[1].y, g_mat_view[2].y, 0.0,
		g_mat_view[0].z, g_mat_view[1].z, g_mat_view[2].z, 0.0,
		0.0,        0.0,        0.0,        1.0
	);

	v_tex0 = g_texcoords.st;

	gl_Position = g_mat_mvp * inverse * g_vertex;

}


#section .frag

void main()
{

	gl_FragColor = diffuse * texture2D(g_tex0_2d, v_tex0);

}

