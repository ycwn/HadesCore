

#include <core.glsl>
#include <tonemap.glsl>

#version 100
#shader  planet::surface
#stage   500

#faces front
#depth lequal


#section .vert, .frag

varying vec3 v_normal;
varying vec3 v_right;
varying vec3 v_up;

varying vec3 v_tile;
varying vec3 v_texcoords;
varying vec3 v_vertex;


#section .vert

void main()
{

	v_normal = vec3(g_mat_model * vec4(g_vertex.xyz, 0.0));
	v_right  = vec3(g_mat_model * vec4(g_normal.xyz, 0.0));
	v_up     = cross(v_normal, v_right);

	v_tile      = g_vertex.xyz;
	v_texcoords = vec3(g_texcoords.st, 1.0);
	v_vertex    = vec3(g_mat_model * g_vertex);

	gl_Position = g_mat_mvp * g_vertex;

}


#section .frag

void main()
{

// tile.xy 0:0, type ocean,   coords = +v_texcoords.zxy
// tile.xy 0:1, type glacier, coords = -v_texcoords.zxy
// tile.xy 1:0, type grass,   coords = +v_texcoords.xzy
// tile.xy 1:1, type desert,  coords = -v_texcoords.xzy

	vec4 tile   = textureCube(g_tex0_cube, v_tile);
	vec3 type   = step(0.5, tile.stp);
	vec3 coords = mix(v_texcoords.zxy, v_texcoords.xzy, type.s) * (1.0 - 2.0 * type.t);

	vec4 color = textureCube(g_tex1_cube, coords); // Surface Detail: RGBS
	vec4 bump  = textureCube(g_tex2_cube, coords); // Surface BumpGloss: UVWE

	vec3 nsurf  = mix(vec3(0.0, 0.0, 1.0), normalize(2.0 * bump.xyz - 1.0), 1.5 * tile.q);
	vec3 normal =
		normalize(v_right)  * nsurf.x +
		normalize(v_up)     * nsurf.y +
		normalize(v_normal) * nsurf.z;

	vec3 light = normalize(g_light0_position.xyz - v_vertex);
	vec3 eye   = normalize(g_eye_position.xyz - v_vertex);
	vec3 halfv = normalize(light + eye);

	float cosNL =     max(dot(normal, light), 0.0);
	float cosNH = pow(max(dot(normal, halfv),  0.0), 96.0 * bump.q);

	gl_FragColor.rgb = tonemap_hdr(color.rgb * cosNL + color.aaa * cosNH);
	gl_FragColor.a   = 1.0;

}

