

#include <core.glsl>

#shader suzanne
#stage  500

#render    ambience
#vertex    V3T2Q4
#primitive triangles
#raster    both
#depth     lequal


VARIABLE(0) vec4 gr_TexCoords;
VARIABLE(1) flat uint texunit;


#section .vert

void main()
{
	gl_Position    = g_object.mat_proj * g_object.mat_view * g_object.mat_model * vec4(2.0 * g_vertex.xyz - 1.0, 1.0);
	gl_Position.x *= g_shader.screen.w;
	gr_TexCoords   = g_texcoords;

	texunit = 0;

}


#section .frag

OUTPUT(0) vec4 fr_Color;


void main()
{

	fr_Color = texture(g_surfaces[texunit], gr_TexCoords.st);

}

