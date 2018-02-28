

#ifndef GR_SHADER_H
#define GR_SHADER_H


typedef struct gr_shader {

	gr_vertexformat   vf;
	gr_renderpass    *rp;
	gr_uniformbuffer  ub;

	int stage;

	VkPipeline pipeline;

	VkShaderModule vertex;
	VkShaderModule tessctrl;
	VkShaderModule tesseval;
	VkShaderModule geometry;
	VkShaderModule fragment;
	VkShaderModule compute;

	V4 args[GPU_SHADER_ARGUMENTS];

	list       node;
	const char name[];

} gr_shader;



void gr_shader_create(graphics *gr);
void gr_shader_destroy();

gr_shader *gr_shader_new( const char *name);
gr_shader *gr_shader_load(const char *file);
void       gr_shader_del(gr_shader *s);

gr_shader *gr_shader_find(const char *name);


#endif

