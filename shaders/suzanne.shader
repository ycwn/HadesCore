

#include <core.glsl>

#shader suzanne
#stage  500

#render    ambience
#vertex    V3T2Q4
#primitive triangles
#raster    both
#depth     lequal


VARIABLE(0) vec4 gr_TexCoords;
VARIABLE(1) vec4 gr_Normal;
VARIABLE(2) flat uint texunit;


#section .vert

void main()
{
	mat4 mv  = g_object.mat_view * g_object.mat_model;
	mat4 mvp = g_object.mat_proj * mv;

	gl_Position    = mvp * vec4(2.0 * g_vertex.xyz - 1.0, 1.0);
	gl_Position.x *= g_shader.screen.w;
	gr_TexCoords   = g_texcoords;
	gr_Normal      = /*mv */ vec4(g_tangent.xyz, 0.0);

	texunit = 4;

}


#section .frag

OUTPUT(0) vec4 fr_Color;
//OUTPUT(1) vec4 fr_Normal;


void main()
{

	fr_Color  = texture(g_surfaces[texunit], gr_TexCoords.st);
//	fr_Normal = gr_Normal;

}

