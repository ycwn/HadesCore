

#include <core.glsl>
#include <tonemap.glsl>

#version 100
#shader  skybox::ambient
#stage   525

#faces front, back
#depth lequal
#write depth:off


#section .vert, .frag

varying vec3 v_texcoords;


#section .vert

void main()
{

	vec4 position = g_mat_mvp * vec4(g_vertex.xyz * 1000.0, 0.0);

	v_texcoords = g_vertex.xyz;
	gl_Position = position.xyzz;

}


#section .frag

void main()
{

	vec4 color = textureCube(g_tex0_cube, v_texcoords) * g_frag_v0;

	gl_FragColor.rgb = tonemap_hdr(color.rgb);
	gl_FragColor.a   = color.a;

}
