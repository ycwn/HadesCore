

#include <core.glsl>

#version 100
#shader  particle::gpu
#stage   475;

#blend add, src-alpha, one
#faces front
#depth lequal
#write depth:off


#section .vert, .fragment

varying vec4 v_color;

#define x0 g_vertex.xyz  // Initial position
#define u0 g_normal.xyz  // Initial velocity
#define t0 g_texcoords   // Life and time0

#define grav    g_frag_v0.xyz  // Gravity
#define color0  g_frag_v1      // Initial color
#define color1  g_frag_v2      // Final color
#define size    g_frag_v3      // Initial size, Final size, time, time^2


#section .vert

void main()
{

	float t = size.z - t0.y;
	float k = t / t0.x;

	vec3  pos   = x0 + u0 * t + 0.5 * grav * t * t;
	vec4  color = mix(color0, color1, k);
	float psize = mix(size.x, size.y, k);

	v_color = color;

	gl_PointSize = 2.5 * psize; // gl_Position.w; // TODO: Project to scale distance
	gl_Position  = g_mat_mvp * vec4(pos, 1.0);

}


#section .frag

void main()
{

	gl_FragColor = v_color * texture2D(g_tex0_2d, gl_PointCoord);

}
