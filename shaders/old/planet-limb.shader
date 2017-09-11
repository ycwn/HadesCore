

#include <core.glsl>
#include <math.glsl>
#include <atmosphere.glsl>
#include <tonemap.glsl>

#version 100
#shader  planet::limb
#stage   610

#blend add, one, one
#faces back
#depth lequal
#write depth:off


#section .vert, .frag

varying vec3 v_edge;
varying vec3 v_normal;
varying vec3 v_inscatter;


#section .vert

void main()
{

	vec3 center = vec3(g_mat_model * vec4(0.0, 0.0, 0.0, 1.0));
	vec3 normal = normalize(vec3(g_mat_model * vec4(g_vertex.xyz, 0.0)));
	vec3 light  = normalize(g_light0_position.xyz - center);
	vec3 eye    = g_eye_position.xyz - center;
	vec3 edge   = horizon(eye, normal, RADIUS_PLANET);

	vec3 irradiance;
	vec3 extinction;

	atmo_inscatter(eye, edge, light, irradiance, extinction);

	v_edge      = edge;
	v_normal    = normal;
	v_inscatter = irradiance * extinction;

	gl_Position = g_mat_mvp * vec4(1.025 * g_vertex.xyz, 1.0);

}


#section .frag

void main()
{
	vec3  edge    = normalize(v_edge);
	vec3  normal  = normalize(v_normal);
	float height  = dot(edge, normal);
	float density = 1.0 - saturate((RADIUS_ATMO * height - RADIUS_PLANET) * SCALE_ATMO);

	gl_FragColor.rgb = tonemap_hdr(density * v_inscatter);
	gl_FragColor.a   = 1.0;

}
