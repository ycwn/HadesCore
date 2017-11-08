

#include <core.glsl>

#shader cube
#stage  500

#render    framebuffer
#vertex    V3C3
#primitive triangles
#raster    front


VARIABLE(0) vec3 gr_Color;


#section .vert

void main()
{
	gl_Position    = g_object.mat_proj * g_object.mat_view * g_object.mat_model * g_vertex;
	gl_Position.x *= g_shader.screen.w;
	gr_Color       = g_color.rgb;
}


#section .frag

OUTPUT(0) vec4 fr_Color;


void main()
{

	fr_Color = vec4(gr_Color, 1.0);

}

