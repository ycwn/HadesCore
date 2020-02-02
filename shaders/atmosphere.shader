

#include <core.glsl>

#shader atmosphere
#stage  800

#render    ambience
#vertex    V3T2Q4
#primitive triangles
#raster    both
#depth     lequal


VARIABLE(0) vec3 gr_Normal;

const vec4 zenith  = vec4(0.4, 0.4, 1.0, 0.0);
const vec4 horizon = vec4(1.0, 1.0, 1.0, 0.0);


#section .vert

void main()
{

	mat4 mvp = g_object.mat_proj * g_object.mat_view * g_object.mat_model;

	gl_Position    = mvp * vec4(2.0 * g_vertex.xyz - 1.0, 0.0);
	gl_Position.x *= g_shader.screen.w;
	gl_Position.w  = gl_Position.z;
	gr_Normal      = 2.0 * g_vertex.xyz - 1.0;

}


#section .frag

OUTPUT(0) vec4 fr_Color;
//OUTPUT(1) vec4 fr_Normal;


void main()
{

	fr_Color  = mix(horizon, zenith, pow(max(dot(gr_Normal, vec3(0.0, 1.0, 0.0)), 0.0), 0.5) );
//	fr_Normal = gr_Normal;

}

