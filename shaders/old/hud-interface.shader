

#include <core.glsl>

#version 100
#shader  hud::interface
#stage   900

#blend add, src-alpha, one-minus-src-alpha
#faces front
#write depth:off


#section .vert, .frag

varying vec2 v_tex0;
varying vec4 v_color;

#define metrics g_frag_v0
#define color   g_frag_v1


#section .vert

void main()
{

	v_tex0  = g_texcoords.st;
//	v_color = color;

	gl_Position = vec4(g_vertex.xy * metrics.zw + (1.0 - g_vertex.xy) * metrics.xy, vec2(0.0, 1.0));

}


#section .frag

void main()
{

	gl_FragColor = color * texture2D(g_tex0_2d, v_tex0);

}
