

#include <core.glsl>

#version 100
#shader  geometry::transparent
#stage   575

#blend add, src-alpha, one-minus-src-alpha
#faces front
#depth lequal
#write depth:off


#section .vert, .frag

varying vec4 v_color;
varying vec2 v_tex0;


#section .vert

void main()
{

	v_color = g_frag_v0;// * color;
	v_tex0  = g_texcoords.st;

	gl_Position  = g_mat_mvp * g_vertex;

}


#section .frag

void main()
{

	gl_FragColor = v_color;// * texture2D(texture0, var_TexCoord);

}
