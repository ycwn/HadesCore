

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/blob.h"
#include "core/logger.h"
#include "core/archive.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/pixelformat.h"
#include "gr/vertexformat.h"
#include "gr/surface.h"
#include "gr/renderpass.h"
#include "gr/uniformbuffer.h"
#include "gr/shader.h"


struct metadata {

	char shader_name[1024];
	int  shader_stage;

	char render_pass[1024];

// Input assembler control
	int vertex_format;
	int primitive_topology;
	int primitive_restart;

// Rasterizer control
	int   raster_discard;
	int   raster_cullmode;
	int   raster_depth_bias;
	float raster_depth_bias_slope;
	float raster_depth_bias_const;
	float raster_depth_bias_clamp;

// Depth testing control
	int depth_write;
	int depth_func;

// Stencil test control: front and back
	int stencil_fail[2];
	int stencil_pass[2];
	int stencil_depthfail[2];
	int stencil_func[2];
	int stencil_mask[2];
	int stencil_write[2];
	int stencil_ref[2];

// Color blend control, per attachment
	struct {

		char name[1024];

		int color_mask;

		int blend_color_func;
		int blend_color_src;
		int blend_color_dst;

		int blend_alpha_func;
		int blend_alpha_src;
		int blend_alpha_dst;

	} attachment[GR_ATTACHMENTS_MAX];

	int attachments;

// Master blend control
	int   blend_logic_op;
	float blend_color[4];

};


static list      shaders = LIST_INIT(&shaders, NULL);
static graphics *gfx     = NULL;

static void           reset(  gr_shader *s);
static void           destroy(gr_shader *s);
static bool           parse_metadata(const char *src, size_t len, struct metadata *md);
static bool           parse_spirv(   const char *src, size_t len, u32 *out, size_t *words);
static VkShaderModule compile_spirv( const void *buf, size_t len);
static bool           create_pipeline(gr_shader *s, const struct metadata *md);



void gr_shader_create(graphics *gr)
{

	gfx = gr;

}



void gr_shader_destroy()
{

	while (!list_empty(&shaders))
		gr_shader_del(list_front(&shaders));

	gfx = NULL;

}



gr_shader *gr_shader_new(const char *name)
{

	gr_shader *s = malloc(sizeof(gr_shader) + strlen(name) + 1);

	gr_uniformbuffer_init(&s->ub);

	strcpy((char*)s->name, name);
	reset(s);

	list_init(  &s->node, s);
	list_append(&shaders, &s->node);

	return s;

}



gr_shader *gr_shader_load(const char *file)
{

	archive ar;

	log_i("shader: Loading %s", file);

	if (!ar_open(&ar, file)) {

		log_e("shader: Failed to open '%s'", file);
		return NULL;
	}

	int fd = ar_get(&ar, ".shader");

	if (fd < 0) {

		log_e("shader: Failed to open '%s': invalid format", file);
		return NULL;

	}

	struct metadata  md;
	VkShaderModule   stage[6];
	const char      *stage_name[6]={
		".vert", ".tesc", ".tese", ".geom", ".frag", ".comp"
	};

	if (!parse_metadata(ar_get_data(&ar, fd), ar_get_length(&ar, fd), &md))
		return NULL;

	gr_renderpass *rp = gr_renderpass_find(md.render_pass);

	if (rp == NULL) {

		log_e("shader: Unknown renderpass '%s'", md.render_pass);
		return NULL;

	}

	for (int n=0; n < countof(stage); n++) {

		int fd = ar_get(&ar, stage_name[n]);

		stage[n] = NULL;

		if (fd < 0)
			continue;

		log_i("shader: Compiling section %s", stage_name[n]);

		const void *buf = ar_get_data(&ar, fd);
		size_t      len = ar_get_length(&ar, fd);

		size_t words = 0;
		u32    spirv[(len + sizeof(u32) - 1) / sizeof(u32)];

		if (parse_spirv(buf, len, spirv, &words))
			stage[n] = compile_spirv(spirv, sizeof(u32) * words);

		if (stage[n] == NULL) {

			log_e("shader: Failed");
			return NULL;

		}

	}

	gr_shader *s = gr_shader_new(md.shader_name);

	s->rp    = rp;
	s->stage = md.shader_stage;

	gr_vf_init(&s->vf, md.vertex_format);

	s->vertex   = stage[0];
	s->tessctrl = stage[1];
	s->tesseval = stage[2];
	s->geometry = stage[3];
	s->fragment = stage[4];
	s->compute  = stage[5];

	if (!create_pipeline(s, &md)) {

		log_e("shader: Failed to create pipeline");
		gr_shader_del(s);
		return NULL;

	}

	gpu_uniform_shader ubs;

	ubs.screen = simd4f_create(
		rp->width, rp->height,
		(float)rp->width  / (float)rp->height,
		(float)rp->height / (float)rp->width);

	memcpy(&ubs.arg,      s->args,            sizeof(ubs.arg));
	memcpy(&ubs.textures, s->rp->surface_ids, sizeof(ubs.textures));

	gr_uniformbuffer_commit(&s->ub, &ubs, sizeof(ubs));

	log_i("shader: Done");
	return s;

}



void gr_shader_del(gr_shader *s)
{

	if (s == NULL)
		return;

	list_remove(&s->node);
	gr_uniformbuffer_del(&s->ub);

	destroy(s);
	free(s);

}



gr_shader *gr_shader_find(const char *name)
{

	for (list *s=list_begin(&shaders); s != list_end(&shaders); s = s->next)
		if (!strcmp(LIST_PTR(gr_shader, s)->name, name))
			return s->ptr;

	return NULL;

}



void reset(gr_shader *s)
{

	gr_vf_init(&s->vf, GR_V2);

	s->pipeline = NULL;

	s->vertex   = NULL;
	s->tessctrl = NULL;
	s->tesseval = NULL;
	s->geometry = NULL;
	s->fragment = NULL;
	s->compute  = NULL;

	mzero(s->args);

}



void destroy(gr_shader *s)
{

	if (s->vertex != NULL)
		vkDestroyShaderModule(gfx->vk.gpu, s->vertex, NULL);

	if (s->tessctrl != NULL)
		vkDestroyShaderModule(gfx->vk.gpu, s->tessctrl, NULL);

	if (s->tesseval != NULL)
		vkDestroyShaderModule(gfx->vk.gpu, s->tesseval, NULL);

	if (s->geometry != NULL)
		vkDestroyShaderModule(gfx->vk.gpu, s->geometry, NULL);

	if (s->fragment != NULL)
		vkDestroyShaderModule(gfx->vk.gpu, s->fragment, NULL);

	if (s->compute != NULL)
		vkDestroyShaderModule(gfx->vk.gpu, s->compute, NULL);

	if (s->pipeline != NULL)
		vkDestroyPipeline(gfx->vk.gpu, s->pipeline, NULL);

	reset(s);

}



bool parse_metadata(const char *src, size_t len, struct metadata *md)
{

	char buf[1024] = { 0 };

	memset(md, 0, sizeof(struct metadata));

	md->vertex_format      = -1;
	md->primitive_topology = -1;

	md->depth_func      = VK_COMPARE_OP_MAX_ENUM;
	md->stencil_func[0] = VK_COMPARE_OP_MAX_ENUM;
	md->stencil_func[1] = VK_COMPARE_OP_MAX_ENUM;
	md->blend_logic_op  = VK_LOGIC_OP_MAX_ENUM;

	for (int n=0, m=0; n < len; n++, src++) {

		if (*src != '\n') {

			if (m + 1 >= sizeof(buf))
				goto fail;

			buf[m++] = *src;
			continue;

		}

		char  s[1024];
		int   a, b, c;
		float x, y, z, w;

		if      (sscanf(buf, ".sh %1024s\n", s) >= 1) strcpy(md->shader_name,   s);
		else if (sscanf(buf, ".rt %1024s\n", s) >= 1) strcpy(md->render_pass, s);

		else if (sscanf(buf, ".st %d\n",    &a)     >= 1) { md->shader_stage    = a; }
		else if (sscanf(buf, ".vf %d\n",    &a)     >= 1) { md->vertex_format   = a; }
		else if (sscanf(buf, ".rd %d\n",    &a)     >= 1) { md->raster_discard  = a; }
		else if (sscanf(buf, ".rc %d\n",    &a)     >= 1) { md->raster_cullmode = a; }
		else if (sscanf(buf, ".dw %d\n",    &a)     >= 1) { md->depth_write     = a; }
		else if (sscanf(buf, ".df %d\n",    &a)     >= 1) { md->depth_func      = a; }
		else if (sscanf(buf, ".bl %d\n",    &a)     >= 1) { md->blend_logic_op  = a; }
		else if (sscanf(buf, ".tp %d %d\n", &a, &b) >= 1) { md->primitive_topology   = a; md->primitive_restart    = b; }
		else if (sscanf(buf, ".sf %d %d\n", &a, &b) >= 1) { md->stencil_fail[0]      = a; md->stencil_fail[1]      = b; }
		else if (sscanf(buf, ".sp %d %d\n", &a, &b) >= 1) { md->stencil_pass[0]      = a; md->stencil_pass[1]      = b; }
		else if (sscanf(buf, ".sd %d %d\n", &a, &b) >= 1) { md->stencil_depthfail[0] = a; md->stencil_depthfail[1] = b; }
		else if (sscanf(buf, ".sc %d %d\n", &a, &b) >= 1) { md->stencil_func[0]      = a; md->stencil_func[1]      = b; }
		else if (sscanf(buf, ".sm %d %d\n", &a, &b) >= 1) { md->stencil_mask[0]      = a; md->stencil_mask[1]      = b; }
		else if (sscanf(buf, ".sw %d %d\n", &a, &b) >= 1) { md->stencil_write[0]     = a; md->stencil_write[1]     = b; }
		else if (sscanf(buf, ".sr %d %d\n", &a, &b) >= 1) { md->stencil_ref[0]       = a; md->stencil_ref[1]       = b; }

		else if (sscanf(buf, ".db %d %f %f %f\n", &a, &x, &y, &z) >= 1) {

			md->raster_depth_bias = a;
			md->raster_depth_bias_const = x;
			md->raster_depth_bias_slope = y;
			md->raster_depth_bias_clamp = z;


		} else if (sscanf(buf, ".cc %f %f %f %f\n", &x, &y, &z, &w) >= 1) {

			md->blend_color[0] = x;
			md->blend_color[1] = y;
			md->blend_color[2] = z;
			md->blend_color[3] = z;

		} else {

			int mode = -1;
			int atch = -1;

			if      (sscanf(buf, ".wm %1024s %d\n",       s, &a) >= 1)         mode = 0; // Color masks
			else if (sscanf(buf, ".bc %1024s %d %d %d\n", s, &a, &b, &c) >= 1) mode = 1; // Color blend
			else if (sscanf(buf, ".ba %1024s %d %d %d\n", s, &a, &b, &c) >= 1) mode = 2; // Alpha blend
			else {

				mzero(buf);
				m = 0;
				continue;

			}

			for (int k=0; k < md->attachments; k++)
				if (!strcmp(md->attachment[k].name, s)) {

					atch = k;
					break;

				}

			if (atch < 0) {

				atch = md->attachments++;

				if (atch > GR_ATTACHMENTS_MAX)
					goto fail;

				strcpy(md->attachment[atch].name, s);

				md->attachment[atch].blend_color_func = VK_BLEND_OP_MAX_ENUM;
				md->attachment[atch].blend_alpha_func = VK_BLEND_OP_MAX_ENUM;

			}

			if (mode == 1) {

				md->attachment[atch].blend_color_func = a;
				md->attachment[atch].blend_color_src  = b;
				md->attachment[atch].blend_color_dst  = c;

			} else if (mode == 2) {

				md->attachment[atch].blend_alpha_func = a;
				md->attachment[atch].blend_alpha_src  = b;
				md->attachment[atch].blend_alpha_dst  = c;

			} else
				md->attachment[atch].color_mask = a;

		}

		mzero(buf);
		m = 0;

	}

	return true;

fail:
	log_e("shader: malformed metadata!");
	return false;

}



bool parse_spirv(const char *src, size_t len, u32 *spirv, size_t *words)
{

	char   buf[1024] = { 0 };
	size_t num = 0;
	size_t k   = 0;
	bool   comment = false;

	for (int n=0; n < len; n++, src++) {

		if (src[0] == '/' && src[1] == '/')
			comment = true;

		if (*src != '\n' && *src != ',') {

			if (num + 1 >= sizeof(buf))
				goto fail;

			if (!isspace(*src) && !comment)
				buf[num++] = *src;

			continue;

		}

		if (num > 0)
			spirv[k++] = strtoul(buf, NULL, 0);

		mzero(buf);
		comment = false;
		num       = 0;

	}

	if (num > 0)
		spirv[k++] = strtoul(buf, NULL, 0);

	if (words != NULL)
		*words = k;

	return true;

fail:
	return false;

}



VkShaderModule compile_spirv(const void *buf, size_t len)
{

	if (buf == NULL)
		return NULL;

	VkShaderModuleCreateInfo smci = {
		.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext    =  NULL,
		.flags    = 0,
		.codeSize = len,
		.pCode    = buf
	};

	VkShaderModule sm = NULL;

	if (vkCreateShaderModule(gfx->vk.gpu, &smci, NULL, &sm) != VK_SUCCESS)
		return NULL;

	return sm;

}



bool create_pipeline(gr_shader *s, const struct metadata *md)
{

	VkPipelineShaderStageCreateInfo        pssci[6]                  = { 0 }; //Shader stage control
	VkPipelineVertexInputStateCreateInfo   pvisci                    = { 0 }; //Controls VertexFormat and constant uniforms
	VkPipelineInputAssemblyStateCreateInfo piasci                    = { 0 }; //Controls input primitive type (point, line, trilist, ...)
	VkPipelineViewportStateCreateInfo      pvsci                     = { 0 }; //Viewport control
	VkPipelineRasterizationStateCreateInfo prsci                     = { 0 }; //Rasterizer control: Culling, Wireframe, DepthBias
	VkPipelineMultisampleStateCreateInfo   pmsci                     = { 0 }; //Multisampler control
	VkPipelineDepthStencilStateCreateInfo  pdssci                    = { 0 };
	VkPipelineColorBlendAttachmentState    pcbas[GR_ATTACHMENTS_MAX] = { 0 }; //Color blending control per output buffer: FIXME: MAX_ATTACHMENTS
	VkPipelineColorBlendStateCreateInfo    pcbsci                    = { 0 }; //Global blending control

	int stage_num = 0;

	if (s->vertex != NULL) {

		pssci[stage_num].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pssci[stage_num].stage  = VK_SHADER_STAGE_VERTEX_BIT;
		pssci[stage_num].module = s->vertex;
		pssci[stage_num].pName  = "main";
		stage_num++;

	}

	if (s->tessctrl != NULL) {

		pssci[stage_num].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pssci[stage_num].stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		pssci[stage_num].module = s->tessctrl;
		pssci[stage_num].pName  = "main";
		stage_num++;

	}

	if (s->tesseval != NULL) {

		pssci[stage_num].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pssci[stage_num].stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		pssci[stage_num].module = s->tesseval;
		pssci[stage_num].pName  = "main";
		stage_num++;

	}

	if (s->geometry != NULL) {

		pssci[stage_num].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pssci[stage_num].stage  = VK_SHADER_STAGE_GEOMETRY_BIT;
		pssci[stage_num].module = s->geometry;
		pssci[stage_num].pName  = "main";
		stage_num++;

	}

	if (s->fragment != NULL) {

		pssci[stage_num].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pssci[stage_num].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
		pssci[stage_num].module = s->fragment;
		pssci[stage_num].pName  = "main";
		stage_num++;

	}

	if (s->compute != NULL) {

		pssci[stage_num].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pssci[stage_num].stage  = VK_SHADER_STAGE_COMPUTE_BIT;
		pssci[stage_num].module = s->compute;
		pssci[stage_num].pName  = "main";
		stage_num++;

	}

	VkVertexInputBindingDescription vibd = { 0 };

	vibd.binding   = 0;
	vibd.stride    = s->vf.stride;
	vibd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription viad[16];
	uint                              viad_num = gr_vf_build_descriptors(&s->vf, &viad[0]);

	pvisci.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pvisci.vertexBindingDescriptionCount   = 1;
	pvisci.vertexAttributeDescriptionCount = viad_num;
	pvisci.pVertexBindingDescriptions      = &vibd;
	pvisci.pVertexAttributeDescriptions    = &viad[0];

	watch("%d", vibd.stride);
	watch("%d", pvisci.vertexAttributeDescriptionCount);

	piasci.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	piasci.topology               = md->primitive_topology;
	piasci.primitiveRestartEnable = md->primitive_restart;

	VkViewport viewport;
	VkRect2D   scissor;

	viewport.x        = 0.0f;
	viewport.y        = 0.0f;
	viewport.width    = (float)s->rp->width;
	viewport.height   = (float)s->rp->height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.offset.x      = 0;
	scissor.offset.y      = 0;
	scissor.extent.width  = s->rp->width;
	scissor.extent.height = s->rp->height;

	pvsci.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pvsci.viewportCount = 1;
	pvsci.pViewports    = &viewport;
	pvsci.scissorCount  = 1;
	pvsci.pScissors     = &scissor;

	prsci.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	prsci.depthClampEnable        = VK_FALSE;
	prsci.rasterizerDiscardEnable = md->raster_discard;
	prsci.polygonMode             = VK_POLYGON_MODE_FILL;
	prsci.lineWidth               = 1.0f;
	prsci.cullMode                = md->raster_cullmode;
	prsci.frontFace               = VK_FRONT_FACE_CLOCKWISE;
	prsci.depthBiasEnable         = md->raster_depth_bias;
	prsci.depthBiasConstantFactor = md->raster_depth_bias_const;
	prsci.depthBiasSlopeFactor    = md->raster_depth_bias_slope;
	prsci.depthBiasClamp          = md->raster_depth_bias_clamp;

	pmsci.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pmsci.sampleShadingEnable  = VK_FALSE;
	pmsci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	const bool depth_test    = md->depth_func      != VK_COMPARE_OP_MAX_ENUM;
	const bool stencil_front = md->stencil_func[0] != VK_COMPARE_OP_MAX_ENUM;
	const bool stencil_back  = md->stencil_func[1] != VK_COMPARE_OP_MAX_ENUM;

	pdssci.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pdssci.depthTestEnable       = depth_test? VK_TRUE: VK_FALSE;
	pdssci.depthWriteEnable      = md->depth_write;
	pdssci.depthCompareOp        = depth_test? md->depth_func: VK_COMPARE_OP_NEVER;
	pdssci.depthBoundsTestEnable = VK_FALSE;
	pdssci.minDepthBounds        = 0.0f;
	pdssci.maxDepthBounds        = 1.0f;
	pdssci.stencilTestEnable     = (stencil_front || stencil_back)? VK_TRUE: VK_FALSE;
	pdssci.front.failOp          = stencil_front? md->stencil_fail[0]:      VK_STENCIL_OP_KEEP;
	pdssci.front.passOp          = stencil_front? md->stencil_pass[0]:      VK_STENCIL_OP_KEEP;
	pdssci.front.depthFailOp     = stencil_front? md->stencil_depthfail[0]: VK_STENCIL_OP_KEEP;
	pdssci.front.compareOp       = stencil_front? md->stencil_func[0]:      VK_COMPARE_OP_NEVER;
	pdssci.front.compareMask     = md->stencil_mask[0];
	pdssci.front.writeMask       = md->stencil_write[0];
	pdssci.front.reference       = md->stencil_ref[0];
	pdssci.back.failOp           = stencil_back? md->stencil_fail[1]:      VK_STENCIL_OP_KEEP;
	pdssci.back.passOp           = stencil_back? md->stencil_pass[1]:      VK_STENCIL_OP_KEEP;
	pdssci.back.depthFailOp      = stencil_back? md->stencil_depthfail[1]: VK_STENCIL_OP_KEEP;
	pdssci.back.compareOp        = stencil_back? md->stencil_func[1]:      VK_COMPARE_OP_NEVER;
	pdssci.back.compareMask      = md->stencil_mask[1];
	pdssci.back.writeMask        = md->stencil_write[1];
	pdssci.back.reference        = md->stencil_ref[1];

//FIXME: Set blend state per attachment [[ATTACHMENTS REQUIRED]]
	pcbas[0].colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	pcbas[0].blendEnable         = VK_FALSE;
	pcbas[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	pcbas[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	pcbas[0].colorBlendOp        = VK_BLEND_OP_ADD;
	pcbas[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	pcbas[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	pcbas[0].alphaBlendOp        = VK_BLEND_OP_ADD;

	const bool blend_logic_op = md->blend_logic_op != VK_LOGIC_OP_MAX_ENUM;

	pcbsci.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pcbsci.logicOpEnable     = blend_logic_op? VK_TRUE: VK_FALSE;
	pcbsci.logicOp           = blend_logic_op? md->blend_logic_op: VK_LOGIC_OP_COPY;
	pcbsci.attachmentCount   = 1;//countof(pcbas);
	pcbsci.pAttachments      = &pcbas[0];
	pcbsci.blendConstants[0] = md->blend_color[0];
	pcbsci.blendConstants[1] = md->blend_color[1];
	pcbsci.blendConstants[2] = md->blend_color[2];
	pcbsci.blendConstants[3] = md->blend_color[3];

	VkGraphicsPipelineCreateInfo gpci = {};

	gpci.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gpci.stageCount          = stage_num;
	gpci.pStages             = &pssci[0];
	gpci.pVertexInputState   = &pvisci;
	gpci.pInputAssemblyState = &piasci;
	gpci.pViewportState      = &pvsci;
	gpci.pRasterizationState = &prsci;
	gpci.pMultisampleState   = &pmsci;
	gpci.pDepthStencilState  = &pdssci;
	gpci.pColorBlendState    = &pcbsci;
	gpci.pDynamicState       = NULL; //FIXME: Set Dynamic state here
	gpci.layout              = gfx->vk.pipeline_layout;
	gpci.renderPass          = s->rp->renderpass;
	gpci.subpass             = 0;
	gpci.basePipelineHandle  = NULL;
	gpci.basePipelineIndex   = -1;

	if (vkCreateGraphicsPipelines(gfx->vk.gpu, NULL, 1, &gpci, NULL, &s->pipeline) != VK_SUCCESS) {

		s->pipeline = NULL;
		return false;

	}

	return true;

}

