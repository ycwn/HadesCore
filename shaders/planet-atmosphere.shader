

#include <core.glsl>
#include <math.glsl>
#include <atmosphere.glsl>
#include <tonemap.glsl>

#version 100
#shader  planet::atmosphere
#stage   600

#blend add, one, src-alpha
#faces front
#depth lequal
#write depth:off


#section .vert, .frag

varying float v_attenuation;
varying vec3  v_inscatter;


#section .vertex

void main()
{

	vec3 center = vec3(g_mat_model * vec4(0.0, 0.0, 0.0, 1.0));
	vec3 normal = normalize(vec3(g_mat_model * vec4(g_vertex.xyz, 0.0)));
	vec3 light  = normalize(g_light0_position.xyz - center);
	vec3 eye    = g_eye_position.xyz - center;

	vec3 irradiance;
	vec3 extinction;

	atmo_inscatter(eye, normal, light, irradiance, extinction);

	v_inscatter   = irradiance * extinction; // Modulate with extinction to make the terminator more dramatic
	v_attenuation = length(extinction);

	gl_Position = g_mat_mvp * g_vertex;

}


#section .frag

void main()
{

	gl_FragColor.rgb = tonemap_hdr(v_inscatter);
	gl_FragColor.a   = v_attenuation;

}
