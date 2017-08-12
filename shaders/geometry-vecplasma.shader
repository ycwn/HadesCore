

#include <core.glsl>
#include <plasma.glsl>

#version 100
#shader  geometry::vecplasma
#stage   575

#blend add, src-alpha, one
#faces front
#depth lequal
#write depth:off

#//texture 0, "textures/shader/gradient3.dds"


#section .vert, .frag

varying vec3 v_norm;
varying vec2 v_tex;

#define wave_Ax g_frag_v0
#define wave_Ay g_frag_v1
#define wave_Az g_frag_v2
#define wave_Bx g_frag_v3
#define wave_By g_frag_v4
#define wave_Bz g_frag_v5
#define wave_C  g_frag_v6
#define wave_D  g_frag_v7

#define time g_timer.x


#section .vert

void main()
{

	v_norm = g_normal.xyz;
	v_tex  = g_texcoords.st;

	gl_Position = g_mat_mvp * g_vertex;

}


#section .frag

void main()
{

	float w  = 0.5 * plasma(v_norm, time, wave_Ax, wave_Ay, wave_Az, wave_Bx, wave_By, wave_Bz, wave_C, wave_D) + 0.5;

	gl_FragColor.rgb = texture2D(g_tex0_2d, vec2(wave_D.x * w, wave_D.z)).rgb;
	gl_FragColor.a   = mix(w, 1.0, wave_D.x);

}
