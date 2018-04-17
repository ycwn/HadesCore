

#include <core.glsl>

#shader scanline
#stage  999

#render    postprocess
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

	vec2 pixel  = gr_TexCoord.st;// + vec2(sin(gr_TexCoord.t * 384.0) * 8.0/1024.0, 0.0);
	vec4 albedo = texture(g_surfaces[g_shader.tex0], pixel);
	vec4 normal = texture(g_surfaces[g_shader.tex1], pixel);
	vec4 glow   = texture(g_surfaces[g_shader.tex2], pixel * 0.5);
	vec3 light  = vec3(0.0, 0.0, -1.0);// vec3(0.577, 0.577, 0.577);

 	fr_Color = albedo * (max(dot(normal.xyz, light), 0.0) + 0.1) + 3.0 * glow;

}

