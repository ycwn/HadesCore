

#include <core.glsl>
#include <math.glsl>

#version 100
#shader  starfield::transparent
#stage   775

#blend add, src-alpha, one-minus-src-alpha
#faces front
#depth lequal
#write depth:off


#section .vert, .frag

varying vec4 v_color;

const float magnitude = 1.50;
const float farplane  = 7500.0;


#section .vert


float apparent_radius(float mu)
{
	const float kappa = 8.168079999606501; // Calulated for Sirius: M=-1.46, R=16.0: K = R * gamma ^ (M / 2.0)
	const float gamma = 2.51188643150958;  // gamma^5.0 = 100.0

	return kappa * pow(gamma, -mu / 2.0);

}



void main()
{

	vec4  position   = g_mat_mvp * vec4(g_vertex.xyz * farplane, 0.0);
	float visibility = step(g_extra.x, magnitude);                    // Brighter stars have smaller magnitude, so the arguments are swapped
	float appearance = apparent_radius(g_extra.x);

	v_color.rgb = vec3(visibility);
	v_color.a   = saturate(appearance);

	gl_Position  = position.xyzz;
	gl_PointSize = appearance;

}


#section .frag

void main()
{

	gl_FragColor = v_color * texture2D(g_tex0_2d, gl_PointCoord);

}

