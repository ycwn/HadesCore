

#include <core.glsl>

#shader suzanne
#stage  500

#render    framebuffer
#vertex    V3T2Q4
#primitive triangles
#raster    front


VARIABLE(0) vec3 gr_Color;


#section .vert

void main()
{
	gl_Position    = g_object.mat_proj * g_object.mat_view * g_object.mat_model * vec4(2.0 * g_vertex.xyz - 1.0, 1.0);
	gl_Position.x *= g_shader.screen.w;
	gr_Color       = 0.5 * g_tangent.rgb + 0.5;
}


#section .frag

OUTPUT(0) vec4 fr_Color;


void main()
{

	fr_Color = vec4(gr_Color, 1.0);

}

