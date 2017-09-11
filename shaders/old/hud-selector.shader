

#include <core.glsl>

#version 100
#shader  hud::selector
#stage   800

#blend add, one, one
#faces front
#depth less
#write depth:off

#//texture 0, "textures/hud-marker.dds"


#section .vert, .frag

varying vec4 v_color;
varying vec2 v_tex0;

#define color g_frag_v0


#section .vert

void main()
{

/*
	mat4 inverse = mat4(
		v0.x, 0.0, v2.x, 0.0,
		 0.0, 1.0,  0.0, 0.0,
		v0.y, 0.0, v2.y, 0.0,
		 0.0, 0.0, 0.0,  1.0
	);
*/
	v_tex0 = g_texcoords.st;

	gl_Position = g_mat_proj * (g_mat_modelview * vec4(0.0, -0.5, 0.0, 1.0) + length(g_mat_modelview[0].xyz) * vec4(g_vertex.xyz, 0.0));

}


#section .frag

void main()
{

	gl_FragColor = texture2D(g_tex0_2d, v_tex0);

}
