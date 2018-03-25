

import hades.core_list;
import hades.core_types;

import hades.graphics_limits;
import hades.graphics_types;
import hades.graphics_graphics;
import hades.graphics_renderpass;
import hades.graphics_uniformbuffer;
import hades.graphics_vertexformat;

import gpuio;


enum { // Generic
	GR_DISABLED = -1,
	GR_ENABLED
}

enum { // Vertex Topology
	GR_POINTS,
	GR_LINES,     GR_LINES_ADJ,     GR_LINE_STRIP,     GR_LINE_STRIP_ADJ,
	GR_TRIANGLES, GR_TRIANGLES_ADJ, GR_TRIANGLE_STRIP, GR_TRIANGLE_STRIP_ADJ, GR_TRIANGLE_FAN,
	GR_PATCHES
}

enum { // Rasterizer face rendering mode
	GR_NONE, GR_FRONT, GR_BACK, GR_FRONT_AND_BACK
}

enum { // Rasterizer polygon fill mode
	GR_POINT, GR_LINE, GR_FILL
}

enum { // Comparator functions
	GR_NEVER, GR_LESS, GR_EQUAL, GR_LEQUAL, GR_GREATER, GR_NOTEQUAL, GR_GEQUAL, GR_ALWAYS
}

enum { // Stencil Ops
	GR_KEEP, GR_RESET, GR_REPLACE, GR_INCR, GR_INCR_WRAP, GR_DECR, GR_DECR_WRAP, GR_INVERT
}

enum { // Color channels
	GR_RGB, GR_ALPHA, GR_RGB_ALPHA
}

enum { // Color components
	GR_R = 1 << 0, GR_G = 1 << 1, GR_B = 1 << 2, GR_A = 1 << 3
}

enum { // Blending equations
	GR_ADD, GR_SUB, GR_SUB_REV, GR_MIN, GR_MAX
}

enum { // Blending functions
	GR_ZERO,
	GR_ONE,
	GR_SRC_COLOR,
	GR_ONE_MINUS_SRC_COLOR,
	GR_DST_COLOR,
	GR_ONE_MINUS_DST_COLOR,
	GR_SRC_ALPHA,
	GR_ONE_MINUS_SRC_ALPHA,
	GR_DST_ALPHA,
	GR_ONE_MINUS_DST_ALPHA,
	GR_CONSTANT_COLOR,
	GR_ONE_MINUS_CONSTANT_COLOR,
	GR_CONSTANT_ALPHA,
	GR_ONE_MINUS_CONSTANT_ALPHA,
	GR_SRC_ALPHA_SATURATE,
	GR_SRC1_COLOR,
	GR_ONE_MINUS_SRC1_COLOR,
	GR_SRC1_ALPHA,
	GR_ONE_MINUS_SRC1_ALPHA
}

enum { // Logic Ops
	GR_CLEAR, GR_AND, GR_AND_REV, GR_COPY,   GR_AND_INV,  GR_NOP,    GR_XOR,  GR_OR,
	GR_NOR,   GR_EQV, GR_INV,     GR_OR_REV, GR_COPY_INV, GR_OR_INV, GR_NAND, GR_SET
}

enum { // Shader stages
	GR_SHADER_VERTEX,   GR_SHADER_TESSELATION_CTRL, GR_SHADER_TESSELATION_EVAL,
	GR_SHADER_GEOMETRY, GR_SHADER_FRAGMENT,         GR_SHADER_COMPUTE,
	GR_SHADER_MAX
}


struct gr_shader {

	VkPipeline                     pipeline;
	VkShaderModule[GR_SHADER_MAX]  shm;
	const(char)*[GR_SHADER_MAX]    entrypoints;

	VkViewport viewport;
	VkRect2D   scissor;

	VkVertexInputBindingDescription                          vibd;
	VkVertexInputAttributeDescription[GR_VERTEXATTR_MAX]     viad;
	VkPipelineShaderStageCreateInfo[GR_SHADER_MAX]           pssci;
	VkPipelineVertexInputStateCreateInfo                     pvisci;
	VkPipelineInputAssemblyStateCreateInfo                   piasci;
	VkPipelineViewportStateCreateInfo                        pvsci;
	VkPipelineRasterizationStateCreateInfo                   prsci;
	VkPipelineMultisampleStateCreateInfo                     pmsci;
	VkPipelineDepthStencilStateCreateInfo                    pdssci;
	VkPipelineColorBlendAttachmentState[GR_ATTACHMENTS_MAX]  pcbas;
	VkPipelineColorBlendStateCreateInfo                      pcbsci;

	uint viad_num;
	uint pssci_num;
	uint pcbas_num;

	int priority;

	V4[GPU_SHADER_ARGUMENTS] args;

	gr_renderpass    *rp;
	gr_vertexformat   vf;
	gr_uniformbuffer  ub;

	list           node;
	const(char)[0] name;

}


extern(C) {

	void gr_shader_create(graphics *gr);
	void gr_shader_destroy();

	gr_shader *gr_shader_new(  const(char) *name, int priority);
	gr_shader *gr_shader_load( const(char) *file, bool finalize);
	gr_shader *gr_shader_parse(const(char) *str, size_t len, bool finalize);
	void       gr_shader_del(gr_shader *s);

	void gr_shader_defaults(        gr_shader *s);
	void gr_shader_vertex_format(   gr_shader *s, int vf);
	void gr_shader_vertex_topology( gr_shader *s, int topology, bool restart);
	void gr_shader_renderpass(      gr_shader *s, gr_renderpass *rp, const(VkRect2D) *scissor);
	void gr_shader_rasterizer_mode( gr_shader *s, int draw, int fill);
	void gr_shader_multisampling(   gr_shader *s);
	void gr_shader_depth_clamp(     gr_shader *s, bool enable);
	void gr_shader_depth_bias(      gr_shader *s, bool enable, float slope, float bias, float clamp);
	void gr_shader_depth_write(     gr_shader *s, bool enable);
	void gr_shader_depth_boundcheck(gr_shader *s, bool enable, float min, float max);
	void gr_shader_depth_func(      gr_shader *s, int op);
	void gr_shader_stencil_op(      gr_shader *s, int faces, int fail, int pass, int depthfail);
	void gr_shader_stencil_func(    gr_shader *s, int faces, int op, int mask);
	void gr_shader_stencil_mask(    gr_shader *s, int faces, int mask);
	void gr_shader_stencil_ref(     gr_shader *s, int faces, int reference);
	void gr_shader_blend_func(      gr_shader *s, int output, int channel, int func, int src, int dst);
	void gr_shader_blend_color_mask(gr_shader *s, int output, int colormask);
	void gr_shader_blend_color_ref( gr_shader *s, float r, float g, float b, float a);
	void gr_shader_blend_logic_op(  gr_shader *s, int op);
	bool gr_shader_upload_spirv(    gr_shader *s, int stage, const(char) *entrypoint, const(ubyte) *buf, size_t len);
	bool gr_shader_compile(         gr_shader *s);

	gr_shader *gr_shader_find(const(char) *name);

}

