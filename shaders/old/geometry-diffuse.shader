

#include <core.glsl>

#version 100
#shader  geometry::diffuse
#stage   500

#faces front
#depth lequal


#section .vert, .frag

varying vec3 v_lvec;
varying vec3 v_wnorm;
varying vec2 v_tex0;

#define color_a g_frag_v0
#define color_d g_frag_v1


#section .vert

void main()
{

	v_lvec  = g_light0_position.xyz - /* directional * */ vec3(g_mat_model * g_vertex);
	v_wnorm = vec3(g_mat_model * vec4(g_normal.xyz, 0.0));
	v_tex0  = g_texcoords.st;

	gl_Position = g_mat_mvp * g_vertex;

}


#section .frag

void main()
{

	vec3 lvec  = normalize(v_lvec);
	vec3 wnorm = normalize(v_wnorm);

	float cosNL = max(dot(lvec, wnorm), 0.0);
	vec4  c_tex = texture2D(g_tex0_2d, v_tex0);

	gl_FragColor = vec4(
		color_a.rgb * g_light_ambient.rgb +
		color_d.rgb * c_tex.rgb * g_light0_diffuse.rgb * cosNL,
		color_d.a   * c_tex.a);

}
