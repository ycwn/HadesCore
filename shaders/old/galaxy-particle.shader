

#include <core.glsl>

#version 100
#shader  galaxy::particle
#stage   550

#blend color-buffer, add, src-alpha, one
#faces front
#depth lequal
#write depth:off


#section .vert, .frag

varying vec4 v_color;


#section .vert

void main()
{

	v_color = g_color;

	gl_Position  = g_mat_mvp * g_vertex;
	gl_PointSize = 2.5 * float(g_extra.x);

}


#section .frag

void main()
{

	gl_FragColor = v_color * texture2D(g_tex0_2d, gl_PointCoord);

}

