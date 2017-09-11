

#include <core.glsl>

#version 100
#shader  geometry::anisotropic
#stage   500

#faces front
#depth lequal


#section .vert, .frag

varying vec3 v_lvec;
varying vec3 v_wnorm;
varying vec3 v_ref;
varying vec2 v_tex0;

#define color_a g_frag_v0
#define color_d g_frag_v1
#define color_s g_frag_v2


#section .vert

void main()
{

	vec3 wvert = vec3(g_mat_model * g_vertex);

	v_lvec  = g_light0_position.xyz - /* directional * */ wvert;
	v_wnorm = vec3(g_mat_model * vec4(g_normal.xyz, 0.0));
	v_ref   = normalize(g_eye_position.xyz - wvert) + normalize(v_lvec);
	v_tex0  = g_texcoords.st;

	gl_Position = g_mat_mvp * g_vertex;

}


#section .frag

void main()
{

	vec3 lvec  = normalize(v_lvec);
	vec3 wnorm = normalize(v_wnorm);
	vec3 ref   = normalize(v_ref);

	float cosNL  = max(dot(lvec, wnorm), 0.0);
	float cosNH  = max(dot(ref,  wnorm), 0.0);
	vec4  c_tex  = texture2D(g_tex0_2d, v_tex0);
	float c_spec = pow(cosNH, color_s.a);

	gl_FragColor = vec4(
		color_a.rgb * g_light_ambient.rgb +
		color_d.rgb * g_light0_diffuse.rgb  * c_tex.rgb * cosNL +
		color_s.rgb * g_light0_specular.rgb * c_tex.a   * c_spec,
		color_d.a);

}
