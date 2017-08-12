

#include <core.glsl>
#include <rotation.glsl>
#include <plasma.glsl>
#include <tonemap.glsl>

#version 100
#shader  planet::clouds
#stage   525

#blend add, src-alpha, one-minus-src-alpha
#faces front
#depth lequal
#write depth:off


#section .vert, .frag

const vec4 wave_Ax = vec4( 1.0,  6.6, -1.55, -4.87);
const vec4 wave_Ay = vec4( 1.0,  4.5, -1.5,  -3.82);
const vec4 wave_Az = vec4( 1.0,  2.5, -1.57, -2.82);
const vec4 wave_Bx = vec4(12.1, 12.1, -1.3,  -1.57);
const vec4 wave_By = vec4( 9.0,  9.0, -1.5,  -1.57);
const vec4 wave_Bz = vec4( 4.3,  4.3, -1.5,  -1.33);
const vec4 wave_C  = vec4( 1.0,  1.0, -2.5,  -1.57);
const vec4 wave_D  = vec4( 0.0,  1.0,  0.0,  -1.57);

const float coverage  = 0.15;
const float sharpness = 0.95;
const float fuzziness = 0.75;

varying vec3  v_normal;
varying vec3  v_vertex;
varying vec2  v_texcoords;
varying float v_phase;

#define time g_timer.x


#section .vert

void main()
{

	vec3 pos = yrotate3(-0.0125 * time) * g_vertex.xyz;

	v_normal    = vec3(g_mat_model * vec4(pos, 0.0));
	v_vertex    = vec3(g_mat_model * vec4(pos, 1.0));
	v_texcoords = g_texcoords.st;
	v_phase     = plasma(g_vertex.xyz, 0.035 * time, wave_Ax, wave_Ay, wave_Az, wave_Bx, wave_By, wave_Bz, wave_C, wave_D) * 0.5 + 0.5;

	gl_Position = g_mat_mvp * vec4(pos * 1.01, 1.0);

}


#section .frag

void main()
{

	vec3  normal = normalize(v_normal);
	vec3  light  = normalize(g_light0_position.xyz - v_vertex);
	float noise  = dot(textureCube(g_tex1_cube, vec3(v_texcoords, 1.0)).rgb, vec3(1.0 / 3.0));

	float cosNL   = max(dot(normal, light), 0.0);
	float density = 1.0 - pow(sharpness, 255.0 * max(noise - (1.0 - coverage) * v_phase, 0.0));
	vec3  color   = fuzziness * vec3(density) * cosNL;

	gl_FragColor.rgb = tonemap_hdr(color);
	gl_FragColor.a   = density;

}

