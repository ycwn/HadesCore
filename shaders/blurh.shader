

#include <core.glsl>

#shader blurh
#stage  997

#render    blurh
#vertex    V2
#primitive triangles
#raster    both
//#blend     0, add, src-alpha, one-minus-src-alpha


VARIABLE(0) vec4 gr_TexCoord;


#section .vert

void main()
{
	gl_Position = vec4(g_vertex.xy * 2.0 - 1.0, 0.0, 1.0);
	gr_TexCoord = vec4(g_vertex.xy * 2.0, 0.0, 1.0);

}


#section .frag

OUTPUT(0) vec4 fr_Color;


vec4 foo(vec4 x) { return 4.0 * x * step(vec4(0.75, 0.75, 0.75, 0.75), x); }


void main()
{

	vec2 pixel = gr_TexCoord.st;// + vec2(sin(gr_TexCoord.t * 384.0) * 8.0/1024.0, 0.0);
	vec2 delta = vec2(1.5 / 1024.0, 0.0);
	vec4 albedo0 = foo(texture(g_surfaces[g_shader.tex0], pixel - 4.0 * delta));
	vec4 albedo1 = foo(texture(g_surfaces[g_shader.tex0], pixel - 3.0 * delta));
	vec4 albedo2 = foo(texture(g_surfaces[g_shader.tex0], pixel - 2.0 * delta));
	vec4 albedo3 = foo(texture(g_surfaces[g_shader.tex0], pixel - 1.0 * delta));
	vec4 albedo4 = foo(texture(g_surfaces[g_shader.tex0], pixel));
	vec4 albedo5 = foo(texture(g_surfaces[g_shader.tex0], pixel + 1.0 * delta));
	vec4 albedo6 = foo(texture(g_surfaces[g_shader.tex0], pixel + 2.0 * delta));
	vec4 albedo7 = foo(texture(g_surfaces[g_shader.tex0], pixel + 3.0 * delta));
	vec4 albedo8 = foo(texture(g_surfaces[g_shader.tex0], pixel + 4.0 * delta));

 	fr_Color = (albedo0 + albedo1 + albedo2 + albedo3 + albedo4 + albedo5 + albedo6 + albedo7 + albedo8) * 0.1111;

}

