

#include <core.glsl>

#shader blurhv
#stage  998

#render    blurhv
#vertex    V2
#primitive triangles
#raster    both
//#blend     0, add, src-alpha, one-minus-src-alpha


VARIABLE(0) vec4 gr_TexCoord;


#section .vert

void main()
{
	gl_Position = vec4(g_vertex.xy * 2.0 - 1.0, 0.0, 1.0);
	gr_TexCoord = vec4(g_vertex.xy, 0.0, 1.0);

}


#section .frag

OUTPUT(0) vec4 fr_Color;


void main()
{

	vec2 pixel = gr_TexCoord.st;// + vec2(sin(gr_TexCoord.t * 384.0) * 8.0/1024.0, 0.0);
	vec2 delta = vec2(0.0, 1.5 / 768.0);
	vec4 albedo0 = texture(g_surfaces[g_shader.tex0], pixel - 4.0 * delta);
	vec4 albedo1 = texture(g_surfaces[g_shader.tex0], pixel - 3.0 * delta);
	vec4 albedo2 = texture(g_surfaces[g_shader.tex0], pixel - 2.0 * delta);
	vec4 albedo3 = texture(g_surfaces[g_shader.tex0], pixel - 1.0 * delta);
	vec4 albedo4 = texture(g_surfaces[g_shader.tex0], pixel);
	vec4 albedo5 = texture(g_surfaces[g_shader.tex0], pixel + 1.0 * delta);
	vec4 albedo6 = texture(g_surfaces[g_shader.tex0], pixel + 2.0 * delta);
	vec4 albedo7 = texture(g_surfaces[g_shader.tex0], pixel + 3.0 * delta);
	vec4 albedo8 = texture(g_surfaces[g_shader.tex0], pixel + 4.0 * delta);

 	fr_Color = (albedo0 + albedo1 + albedo2 + albedo3 + albedo4 + albedo5 + albedo6 + albedo7 + albedo8) * 0.111;

}

