

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/blob.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/pixelformat.h"
#include "gr/vertexformat.h"
#include "gr/surface.h"
#include "gr/renderpass.h"
#include "gr/uniformbuffer.h"
#include "gr/shader.h"


static list      shaders = LIST_INIT(&shaders, NULL);
static graphics *gfx     = NULL;

static void reset(  gr_shader *s);
static void destroy(gr_shader *s);


#define SHADER_PARSER_ERROR(msg, ...)                          \
	do {                                                   \
		log_e("shader: error: " msg, ## __VA_ARGS__);  \
		goto fail;                                     \
	} while (0)


#define SHADER_PARSER_CHECK(sh)                                              \
	do {                                                                 \
		if (sh == NULL)                                              \
			SHADER_PARSER_ERROR("No active shader definition");  \
	} while (0)



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



gr_shader *gr_shader_new(const char *name, int priority)
{

	gr_shader *s = malloc(sizeof(gr_shader) + strlen(name) + 1);

	gr_uniformbuffer_init(&s->ub);

	strcpy((char*)s->name, name);
	reset(s);

	s->priority = priority;

	list_init(  &s->node, s);
	list_append(&shaders, &s->node);

	return s;

}



gr_shader *gr_shader_load(const char *file, bool finalize)
{

	log_i("shader: Loading %s", file);

	int id = blob_open(file, BLOB_REV_LAST);

	if (id < 0) {

		log_e("shader: Failed to open '%s'", file);
		return NULL;
	}

	gr_shader *s = gr_shader_parse(blob_get_data(id), blob_get_length(id), finalize);

	if (s == NULL)
		return NULL;

	log_i("shader: Done");

	return s;
}



gr_shader *gr_shader_parse(const char *src, size_t len, bool finalize)
{

	int         ll = 0;
	int         n  = 0;
	const char *s;
	const char *ss;

	char  t[len + 1];
	int   a, b, c, d, e;
	float x, y, z, w;

	gr_shader     *sh = NULL;
	gr_renderpass *rp = NULL;

	for (n=0; n < len; n += ll + 1) {

		s  = src + n;
		ll = strcspn(s, "\n");

		if (sscanf(s, ".sh %s %d", t, &a) >= 1) {

			if (sh != NULL)
				SHADER_PARSER_ERROR("Unexpected shader declaration");

			if (gr_shader_find(t) != NULL)
				SHADER_PARSER_ERROR("Shader '%s' already exists", t);

			sh = gr_shader_new(t, a);

		} else if (sscanf(s, ".vf %d", &a) >= 1) {

			SHADER_PARSER_CHECK(sh);
			gr_shader_vertex_format(sh, a);

		} else if (sscanf(s, ".vt %d %d", &a, &b) >= 1) {

			SHADER_PARSER_CHECK(sh);
			gr_shader_vertex_topology(sh, a, b);

		} else if (sscanf(s, ".rp %s", t) >= 1) {

			SHADER_PARSER_CHECK(sh);

			rp = gr_renderpass_find(t);

			if (rp == NULL)
				SHADER_PARSER_ERROR("Renderpass '%s' does not exist", t);

			gr_shader_renderpass(sh, rp, NULL);

		} else if (sscanf(s, ".rm %d %d", &a, &b) >= 1) {

			SHADER_PARSER_CHECK(sh);
			gr_shader_rasterizer_mode(sh, a, b);

		} else if (sscanf(s, ".db %d %f %f %f", &a, &x, &y, &z) >= 1) {

			SHADER_PARSER_CHECK(sh);
			gr_shader_depth_bias(sh, a, x, y, z);

		} else if (sscanf(s, ".dw %d", &a) >= 1) {

			SHADER_PARSER_CHECK(sh);
			gr_shader_depth_write(sh, a);

		} else if (sscanf(s, ".df %d", &a) >= 1) {

			SHADER_PARSER_CHECK(sh);
			gr_shader_depth_func(sh, a);

		} else if (sscanf(s, ".so %d %d %d %d", &a, &b, &c, &d) >= 1) {

			SHADER_PARSER_CHECK(sh);
			gr_shader_stencil_op(sh, a, b, c, d);

		} else if (sscanf(s, ".sf %d %d %u", &a, &b, &c) >= 1) {

			SHADER_PARSER_CHECK(sh);
			gr_shader_stencil_func(sh, a, b, c);

		} else if (sscanf(s, ".sw %d %u", &a, &b) >= 1) {

			SHADER_PARSER_CHECK(sh);
			gr_shader_stencil_mask(sh, a, b);

		} else if (sscanf(s, ".cm %d %d", &a, &b) >= 1) {

			SHADER_PARSER_CHECK(sh);
			gr_shader_blend_color_mask(sh, a, b);

		} else if (sscanf(s, ".sr %d %u", &a, &b) >= 1) {

			SHADER_PARSER_CHECK(sh);
			gr_shader_stencil_ref(sh, a, b);

		} else if (sscanf(s, ".bf %d %d %d %d %d", &a, &b, &c, &d, &e) >= 1) {

			SHADER_PARSER_CHECK(sh);
			gr_shader_blend_func(sh, a, b, c, d, e);

		} else if (sscanf(s, ".op %d", &a) >= 1) {

			SHADER_PARSER_CHECK(sh);
			gr_shader_blend_logic_op(sh, a);

		} else if (sscanf(s, ".cc %f %f %f %f", &x, &y, &z, &w) >= 1) {

			SHADER_PARSER_CHECK(sh);
			gr_shader_blend_color_ref(sh, x, y, z, w);

		} else if (sscanf(s, ".sp %d %d", &a, &b) >= 1) {

			SHADER_PARSER_CHECK(sh);

			u32 spirv[b];

			s += ll;

			for (int k=0; k < b;) {

				if (*s == '\n') {

					n += ll + 1;

					if (n >= len)
						SHADER_PARSER_ERROR("Unexpected EOF!");

					s  = src + n;
					ll = strcspn(s, "\n");

				} else if (*s == ',') {

					s++;
					continue;

				}

				spirv[k++] = strtoul(s, (char**)&ss, 0);

				if (s == ss) {

					n = s - src;
					SHADER_PARSER_ERROR("Failed to parse SPIRV entry");

				}

				s = ss;

			}

			gr_shader_upload_spirv(sh, a, NULL, spirv, sizeof(spirv));

		} else
			log_w("shader: warning: Unknown directive: '%.*s'", ll, s);

	}

	SHADER_PARSER_CHECK(sh);

	if (finalize && !gr_shader_compile(sh))
		SHADER_PARSER_ERROR("Compilation failed");

	return sh;

fail:;
	int line=1, col=0;

	for (int m=0; m < n; m++)
		if (src[m] == '\n') {

			line++;
			col = 0;

		} else
			col++;

	log_e("shader: SPIRV parser stopped at %d:%d", line, col);
	gr_shader_del(sh);

	return NULL;

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



void gr_shader_defaults(gr_shader *s)
{

	s->pvisci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	s->piasci.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	s->pvsci.sType  = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	s->prsci.sType  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	s->pmsci.sType  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	s->pdssci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	s->pcbsci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

	gr_shader_vertex_format(   s, GR_V2);
	gr_shader_vertex_topology( s, GR_TRIANGLES, false);
	gr_shader_renderpass(      s, NULL, NULL);
	gr_shader_rasterizer_mode( s, GR_FRONT, GR_FILL);
	gr_shader_multisampling(   s);
	gr_shader_depth_clamp(     s, false);
	gr_shader_depth_bias(      s, false, 0.0f, 0.0f, 0.0f);
	gr_shader_depth_write(     s, true);
	gr_shader_depth_boundcheck(s, false, 0.0f, 1.0f);
	gr_shader_depth_func(      s, GR_DISABLED);
	gr_shader_stencil_op(      s, GR_FRONT_AND_BACK, GR_KEEP, GR_KEEP, GR_KEEP);
	gr_shader_stencil_func(    s, GR_FRONT_AND_BACK, GR_DISABLED, 0);
	gr_shader_stencil_mask(    s, GR_FRONT_AND_BACK, 0);
	gr_shader_stencil_ref(     s, GR_FRONT_AND_BACK, 0);
	gr_shader_blend_func(      s, 0, GR_RGB_ALPHA, GR_DISABLED, GR_ONE, GR_ZERO);
	gr_shader_blend_color_mask(s, 0, GR_R | GR_G | GR_B | GR_A);
	gr_shader_blend_color_ref( s, 0.0f, 0.0f, 0.0f, 0.0f);
	gr_shader_blend_logic_op(  s, GR_DISABLED);
	gr_shader_upload_spirv(    s, GR_SHADER_VERTEX,           "main", NULL, 0);
	gr_shader_upload_spirv(    s, GR_SHADER_TESSELATION_CTRL, "main", NULL, 0);
	gr_shader_upload_spirv(    s, GR_SHADER_TESSELATION_EVAL, "main", NULL, 0);
	gr_shader_upload_spirv(    s, GR_SHADER_GEOMETRY,         "main", NULL, 0);
	gr_shader_upload_spirv(    s, GR_SHADER_FRAGMENT,         "main", NULL, 0);
	gr_shader_upload_spirv(    s, GR_SHADER_COMPUTE,          "main", NULL, 0);

}



void gr_shader_vertex_format(gr_shader *s, int vf)
{

	gr_vf_init(&s->vf, vf);

	s->vibd.binding   = 0;
	s->vibd.stride    = s->vf.stride;
	s->vibd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	s->viad_num = gr_vf_build_descriptors(&s->vf, &s->viad[0]);

	s->pvisci.vertexBindingDescriptionCount   = 1;
	s->pvisci.vertexAttributeDescriptionCount = s->viad_num;
	s->pvisci.pVertexBindingDescriptions      = &s->vibd;
	s->pvisci.pVertexAttributeDescriptions    = &s->viad[0];

}



void gr_shader_vertex_topology(gr_shader *s, int topology, bool restart)
{

	int top =
		(topology == GR_POINTS)?              VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
		(topology == GR_LINES)?               VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
		(topology == GR_LINE_STRIP)?          VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		(topology == GR_TRIANGLES)?           VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
		(topology == GR_TRIANGLE_STRIP)?      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
		(topology == GR_TRIANGLE_FAN)?        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
		(topology == GR_LINES_ADJ)?           VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
		(topology == GR_LINE_STRIP_ADJ)?      VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
		(topology == GR_TRIANGLES_ADJ)?       VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
		(topology == GR_TRIANGLE_STRIP_ADJ)?  VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
		(topology == GR_PATCHES)?             VK_PRIMITIVE_TOPOLOGY_PATCH_LIST: -1;

	if (top < 0)
		return;

	s->piasci.topology               = top;
	s->piasci.primitiveRestartEnable = restart? VK_TRUE: VK_FALSE;

}



void gr_shader_renderpass(gr_shader *s, gr_renderpass *rp, const VkRect2D *scissor)
{

	s->rp = rp;;

	if (rp != NULL) {

		s->viewport.x        = 0.0f;
		s->viewport.y        = 0.0f;
		s->viewport.width    = (float)rp->width;
		s->viewport.height   = (float)rp->height;
		s->viewport.minDepth = 0.0f; //FIXME: Get from renderpass
		s->viewport.maxDepth = 1.0f; //FIXME: Get from renderpass

		if (scissor == NULL) {

			s->scissor.offset.x      = 0;
			s->scissor.offset.y      = 0;
			s->scissor.extent.width  = rp->width;
			s->scissor.extent.height = rp->height;

		} else
			s->scissor = *scissor;

		s->pcbas_num = rp->attachment_colors;

	} else {

		szero(s->viewport);
		szero(s->scissor);

		s->pcbas_num = 0;

	}

}



void gr_shader_rasterizer_mode(gr_shader *s, int draw, int fill)
{

	int cull =
		(draw == GR_DISABLED)?       VK_CULL_MODE_FRONT_AND_BACK:
		(draw == GR_NONE)?           VK_CULL_MODE_FRONT_AND_BACK:
		(draw == GR_FRONT)?          VK_CULL_MODE_BACK_BIT:
		(draw == GR_BACK)?           VK_CULL_MODE_FRONT_BIT:
		(draw == GR_FRONT_AND_BACK)? VK_CULL_MODE_NONE: -1;

	int mode =
		(fill == GR_POINT)? VK_POLYGON_MODE_POINT:
		(fill == GR_LINE)?  VK_POLYGON_MODE_LINE:
		(fill == GR_FILL)?  VK_POLYGON_MODE_FILL: -1;

	if (cull < 0 || mode < 0)
		return;

	s->prsci.rasterizerDiscardEnable = (draw == GR_DISABLED)? VK_TRUE: VK_FALSE;
	s->prsci.polygonMode             = mode;
	s->prsci.cullMode                = cull;
	s->prsci.frontFace               = VK_FRONT_FACE_CLOCKWISE;
	s->prsci.lineWidth               = 1.0f;

}



void gr_shader_multisampling(gr_shader *s)
{

	s->pmsci.sampleShadingEnable  = VK_FALSE;
	s->pmsci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

}



void gr_shader_depth_clamp(gr_shader *s, bool enable)
{

	s->prsci.depthClampEnable = enable? VK_TRUE: VK_FALSE;

}



void gr_shader_depth_bias(gr_shader *s, bool enable, float slope, float bias, float clamp)
{

	s->prsci.depthBiasEnable         = enable? VK_TRUE: VK_FALSE;
	s->prsci.depthBiasConstantFactor = bias;
	s->prsci.depthBiasSlopeFactor    = slope;
	s->prsci.depthBiasClamp          = clamp;

}



void gr_shader_depth_write(gr_shader *s, bool enable)
{

	s->pdssci.depthWriteEnable = enable? VK_TRUE: VK_FALSE;

}



void gr_shader_depth_boundcheck(gr_shader *s, bool enable, float min, float max)
{

	s->pdssci.depthBoundsTestEnable = enable? VK_TRUE: VK_FALSE;
	s->pdssci.minDepthBounds        = min;
	s->pdssci.maxDepthBounds        = max;

}



void gr_shader_depth_func(gr_shader *s, int op)
{

	int cmp =
		(op == GR_DISABLED)? VK_COMPARE_OP_NEVER:
		(op == GR_NEVER)?    VK_COMPARE_OP_NEVER:
		(op == GR_LESS)?     VK_COMPARE_OP_LESS:
		(op == GR_EQUAL)?    VK_COMPARE_OP_EQUAL:
		(op == GR_LEQUAL)?   VK_COMPARE_OP_LESS_OR_EQUAL:
		(op == GR_GREATER)?  VK_COMPARE_OP_GREATER:
		(op == GR_NOTEQUAL)? VK_COMPARE_OP_NOT_EQUAL:
		(op == GR_GEQUAL)?   VK_COMPARE_OP_GREATER_OR_EQUAL:
		(op == GR_ALWAYS)?   VK_COMPARE_OP_ALWAYS: -1;

	if (cmp < 0)
		return;

	s->pdssci.depthTestEnable = (op == GR_DISABLED)? VK_FALSE: VK_TRUE;
	s->pdssci.depthCompareOp  = cmp;

}



void gr_shader_stencil_op(gr_shader *s, int faces, int fail, int pass, int depthfail)
{

	int fail_op =
		(fail == GR_KEEP)?      VK_STENCIL_OP_KEEP:
		(fail == GR_ZERO)?      VK_STENCIL_OP_ZERO:
		(fail == GR_REPLACE)?   VK_STENCIL_OP_REPLACE:
		(fail == GR_INCR)?      VK_STENCIL_OP_INCREMENT_AND_CLAMP:
		(fail == GR_INCR_WRAP)? VK_STENCIL_OP_INCREMENT_AND_WRAP:
		(fail == GR_DECR)?      VK_STENCIL_OP_DECREMENT_AND_CLAMP:
		(fail == GR_DECR_WRAP)? VK_STENCIL_OP_DECREMENT_AND_WRAP:
		(fail == GR_INVERT)?    VK_STENCIL_OP_INVERT: -1;

	int pass_op =
		(pass == GR_KEEP)?      VK_STENCIL_OP_KEEP:
		(pass == GR_ZERO)?      VK_STENCIL_OP_ZERO:
		(pass == GR_REPLACE)?   VK_STENCIL_OP_REPLACE:
		(pass == GR_INCR)?      VK_STENCIL_OP_INCREMENT_AND_CLAMP:
		(pass == GR_INCR_WRAP)? VK_STENCIL_OP_INCREMENT_AND_WRAP:
		(pass == GR_DECR)?      VK_STENCIL_OP_DECREMENT_AND_CLAMP:
		(pass == GR_DECR_WRAP)? VK_STENCIL_OP_DECREMENT_AND_WRAP:
		(pass == GR_INVERT)?    VK_STENCIL_OP_INVERT: -1;

	int depthfail_op =
		(depthfail == GR_KEEP)?      VK_STENCIL_OP_KEEP:
		(depthfail == GR_ZERO)?      VK_STENCIL_OP_ZERO:
		(depthfail == GR_REPLACE)?   VK_STENCIL_OP_REPLACE:
		(depthfail == GR_INCR)?      VK_STENCIL_OP_INCREMENT_AND_CLAMP:
		(depthfail == GR_INCR_WRAP)? VK_STENCIL_OP_INCREMENT_AND_WRAP:
		(depthfail == GR_DECR)?      VK_STENCIL_OP_DECREMENT_AND_CLAMP:
		(depthfail == GR_DECR_WRAP)? VK_STENCIL_OP_DECREMENT_AND_WRAP:
		(depthfail == GR_INVERT)?    VK_STENCIL_OP_INVERT: -1;

	if (fail_op < 0 || pass_op < 0 || depthfail_op < 0)
		return;

	if (faces == GR_FRONT || faces == GR_FRONT_AND_BACK) {

		s->pdssci.front.failOp      = fail_op;
		s->pdssci.front.passOp      = pass_op;
		s->pdssci.front.depthFailOp = depthfail_op;

	}

	if (faces == GR_BACK || faces == GR_FRONT_AND_BACK) {

		s->pdssci.back.failOp      = fail_op;
		s->pdssci.back.passOp      = pass_op;
		s->pdssci.back.depthFailOp = depthfail_op;

	}

}



void gr_shader_stencil_func(gr_shader *s, int faces, int op, int mask)
{

	int cmp =
		(op == GR_DISABLED)? VK_COMPARE_OP_NEVER:
		(op == GR_NEVER)?    VK_COMPARE_OP_NEVER:
		(op == GR_LESS)?     VK_COMPARE_OP_LESS:
		(op == GR_EQUAL)?    VK_COMPARE_OP_EQUAL:
		(op == GR_LEQUAL)?   VK_COMPARE_OP_LESS_OR_EQUAL:
		(op == GR_GREATER)?  VK_COMPARE_OP_GREATER:
		(op == GR_NOTEQUAL)? VK_COMPARE_OP_NOT_EQUAL:
		(op == GR_GEQUAL)?   VK_COMPARE_OP_GREATER_OR_EQUAL:
		(op == GR_ALWAYS)?   VK_COMPARE_OP_ALWAYS: -1;

	if (cmp < 0)
		return;

	s->pdssci.stencilTestEnable = (op == GR_DISABLED)? VK_FALSE: VK_TRUE;

	if (faces == GR_FRONT || faces == GR_FRONT_AND_BACK) {

		s->pdssci.front.compareOp   = cmp;
		s->pdssci.front.compareMask = mask;

	}

	if (faces == GR_BACK || faces == GR_FRONT_AND_BACK) {

		s->pdssci.back.compareOp   = cmp;
		s->pdssci.back.compareMask = mask;

	}

}



void gr_shader_stencil_mask(gr_shader *s, int faces, int mask)
{

	if (faces == GR_FRONT || faces == GR_FRONT_AND_BACK)
		s->pdssci.front.writeMask = mask;

	if (faces == GR_BACK || faces == GR_FRONT_AND_BACK)
		s->pdssci.back.writeMask = mask;

}



void gr_shader_stencil_ref(gr_shader *s, int faces, int reference)
{

	if (faces == GR_FRONT || faces == GR_FRONT_AND_BACK)
		s->pdssci.front.reference = reference;

	if (faces == GR_BACK || faces == GR_FRONT_AND_BACK)
		s->pdssci.back.reference = reference;

}



void gr_shader_blend_func(gr_shader *s, int output, int channel, int func, int src, int dst)
{

	if (output < 0 || output >= GR_ATTACHMENTS_MAX)
		return;

	int eqn =
		(func == GR_DISABLED)? VK_BLEND_OP_ADD:
		(func == GR_ADD)?      VK_BLEND_OP_ADD:
		(func == GR_SUB)?      VK_BLEND_OP_SUBTRACT:
		(func == GR_SUB_REV)?  VK_BLEND_OP_REVERSE_SUBTRACT:
		(func == GR_MIN)?      VK_BLEND_OP_MIN:
		(func == GR_MAX)?      VK_BLEND_OP_MAX: -1;

	int f_src =
		(src == GR_ZERO)?                     VK_BLEND_FACTOR_ZERO:
		(src == GR_ONE)?                      VK_BLEND_FACTOR_ONE:
		(src == GR_SRC_COLOR)?                VK_BLEND_FACTOR_SRC_COLOR:
		(src == GR_ONE_MINUS_SRC_COLOR)?      VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
		(src == GR_DST_COLOR)?                VK_BLEND_FACTOR_DST_COLOR:
		(src == GR_ONE_MINUS_DST_COLOR)?      VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
		(src == GR_SRC_ALPHA)?                VK_BLEND_FACTOR_SRC_ALPHA:
		(src == GR_ONE_MINUS_SRC_ALPHA)?      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
		(src == GR_DST_ALPHA)?                VK_BLEND_FACTOR_DST_ALPHA:
		(src == GR_ONE_MINUS_DST_ALPHA)?      VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
		(src == GR_CONSTANT_COLOR)?           VK_BLEND_FACTOR_CONSTANT_COLOR:
		(src == GR_ONE_MINUS_CONSTANT_COLOR)? VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
		(src == GR_CONSTANT_ALPHA)?           VK_BLEND_FACTOR_CONSTANT_ALPHA:
		(src == GR_ONE_MINUS_CONSTANT_ALPHA)? VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
		(src == GR_SRC_ALPHA_SATURATE)?       VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
		(src == GR_SRC1_COLOR)?               VK_BLEND_FACTOR_SRC1_COLOR:
		(src == GR_ONE_MINUS_SRC1_COLOR)?     VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
		(src == GR_SRC1_ALPHA)?               VK_BLEND_FACTOR_SRC1_ALPHA:
		(src == GR_ONE_MINUS_SRC1_ALPHA)?     VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA: -1;

	int f_dst =
		(dst == GR_ZERO)?                     VK_BLEND_FACTOR_ZERO:
		(dst == GR_ONE)?                      VK_BLEND_FACTOR_ONE:
		(dst == GR_SRC_COLOR)?                VK_BLEND_FACTOR_SRC_COLOR:
		(dst == GR_ONE_MINUS_SRC_COLOR)?      VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
		(dst == GR_DST_COLOR)?                VK_BLEND_FACTOR_DST_COLOR:
		(dst == GR_ONE_MINUS_DST_COLOR)?      VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
		(dst == GR_SRC_ALPHA)?                VK_BLEND_FACTOR_SRC_ALPHA:
		(dst == GR_ONE_MINUS_SRC_ALPHA)?      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
		(dst == GR_DST_ALPHA)?                VK_BLEND_FACTOR_DST_ALPHA:
		(dst == GR_ONE_MINUS_DST_ALPHA)?      VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
		(dst == GR_CONSTANT_COLOR)?           VK_BLEND_FACTOR_CONSTANT_COLOR:
		(dst == GR_ONE_MINUS_CONSTANT_COLOR)? VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
		(dst == GR_CONSTANT_ALPHA)?           VK_BLEND_FACTOR_CONSTANT_ALPHA:
		(dst == GR_ONE_MINUS_CONSTANT_ALPHA)? VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
		(dst == GR_SRC_ALPHA_SATURATE)?       VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
		(dst == GR_SRC1_COLOR)?               VK_BLEND_FACTOR_SRC1_COLOR:
		(dst == GR_ONE_MINUS_SRC1_COLOR)?     VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
		(dst == GR_SRC1_ALPHA)?               VK_BLEND_FACTOR_SRC1_ALPHA:
		(dst == GR_ONE_MINUS_SRC1_ALPHA)?     VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA: -1;

	s->pcbas[output].blendEnable = (func == GR_DISABLED)? VK_FALSE: VK_TRUE;

	if (channel == GR_RGB || channel == GR_RGB_ALPHA) {

		s->pcbas[output].srcColorBlendFactor = f_src;
		s->pcbas[output].dstColorBlendFactor = f_dst;
		s->pcbas[output].colorBlendOp        = eqn;

	}

	if (channel == GR_ALPHA || channel == GR_RGB_ALPHA) {

		s->pcbas[output].srcAlphaBlendFactor = f_src;
		s->pcbas[output].dstAlphaBlendFactor = f_dst;
		s->pcbas[output].alphaBlendOp        = eqn;

	}

}



void gr_shader_blend_color_mask(gr_shader *s, int output, int colormask)
{

	if (output < 0 || output >= GR_ATTACHMENTS_MAX)
		return;

	int mask = 0;

	if (colormask & GR_R) mask |= VK_COLOR_COMPONENT_R_BIT;
	if (colormask & GR_G) mask |= VK_COLOR_COMPONENT_G_BIT;
	if (colormask & GR_B) mask |= VK_COLOR_COMPONENT_B_BIT;
	if (colormask & GR_A) mask |= VK_COLOR_COMPONENT_A_BIT;

	s->pcbas[output].colorWriteMask = mask;

}



void gr_shader_blend_color_ref(gr_shader *s, float r, float g, float b, float a)
{

	s->pcbsci.blendConstants[0] = r;
	s->pcbsci.blendConstants[1] = g;
	s->pcbsci.blendConstants[2] = b;
	s->pcbsci.blendConstants[3] = a;

}



void gr_shader_blend_logic_op(gr_shader* s, int op)
{

	int logic =
		(op == GR_DISABLED)? VK_LOGIC_OP_COPY:
		(op == GR_CLEAR)?    VK_LOGIC_OP_CLEAR:
		(op == GR_AND)?      VK_LOGIC_OP_AND:
		(op == GR_AND_REV)?  VK_LOGIC_OP_AND_REVERSE:
		(op == GR_COPY)?     VK_LOGIC_OP_COPY:
		(op == GR_AND_INV)?  VK_LOGIC_OP_AND_INVERTED:
		(op == GR_NOP)?      VK_LOGIC_OP_NO_OP:
		(op == GR_XOR)?      VK_LOGIC_OP_XOR:
		(op == GR_OR)?       VK_LOGIC_OP_OR:
		(op == GR_NOR)?      VK_LOGIC_OP_NOR:
		(op == GR_EQV)?      VK_LOGIC_OP_EQUIVALENT:
		(op == GR_INV)?      VK_LOGIC_OP_INVERT:
		(op == GR_OR_REV)?   VK_LOGIC_OP_OR_REVERSE:
		(op == GR_COPY_INV)? VK_LOGIC_OP_COPY_INVERTED:
		(op == GR_OR_INV)?   VK_LOGIC_OP_OR_INVERTED:
		(op == GR_NAND)?     VK_LOGIC_OP_NAND:
		(op == GR_SET)?      VK_LOGIC_OP_SET: -1;

	if (logic < 0)
		return;

	s->pcbsci.logicOpEnable = (op != GR_DISABLED)? VK_TRUE: VK_FALSE;
	s->pcbsci.logicOp       = logic;

}



bool gr_shader_upload_spirv(gr_shader *s, int stage, const char *entrypoint, const void *buf, size_t len)
{

	if (stage < 0 || stage >= GR_SHADER_MAX)
		return false;

	if (s->module[stage] != NULL)
		vkDestroyShaderModule(gfx->vk.gpu, s->module[stage], NULL);

	free((char*)s->entrypoints[stage]);

	s->module[stage]      = NULL;
	s->entrypoints[stage] = NULL;

	if (buf == NULL)
		return true;

	VkShaderModuleCreateInfo smci = {
		.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext    = NULL,
		.flags    = 0,
		.codeSize = len,
		.pCode    = buf
	};

	if (vkCreateShaderModule(gfx->vk.gpu, &smci, NULL, &s->module[stage]) != VK_SUCCESS)
		return false;

	if (entrypoint != NULL)
		s->entrypoints[stage] = strdup(entrypoint);

	return true;

}



bool gr_shader_compile(gr_shader *s)
{

	s->pssci_num = 0;

	static const int stages[] = {
		VK_SHADER_STAGE_VERTEX_BIT,
		VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
		VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
		VK_SHADER_STAGE_GEOMETRY_BIT,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		VK_SHADER_STAGE_COMPUTE_BIT
	};

	for (int n=0; n < GR_SHADER_MAX; n++)
		if (s->module[n] != NULL) {

			s->pssci[s->pssci_num].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			s->pssci[s->pssci_num].stage  = stages[n];
			s->pssci[s->pssci_num].module = s->module[n];
			s->pssci[s->pssci_num].pName  = (s->entrypoints[n] != NULL)? s->entrypoints[n]: "main";
			s->pssci_num++;

		}

	s->pvsci.viewportCount = 1;
	s->pvsci.pViewports    = &s->viewport;
	s->pvsci.scissorCount  = 1;
	s->pvsci.pScissors     = &s->scissor;

	s->pcbsci.attachmentCount = s->pcbas_num;
	s->pcbsci.pAttachments    = &s->pcbas[0];

	VkGraphicsPipelineCreateInfo gpci = { 0 };

	gpci.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gpci.stageCount          = s->pssci_num;
	gpci.pStages             = &s->pssci[0];
	gpci.pVertexInputState   = &s->pvisci;
	gpci.pInputAssemblyState = &s->piasci;
	gpci.pViewportState      = &s->pvsci;
	gpci.pRasterizationState = &s->prsci;
	gpci.pMultisampleState   = &s->pmsci;
	gpci.pDepthStencilState  = &s->pdssci;
	gpci.pColorBlendState    = &s->pcbsci;
	gpci.pDynamicState       = NULL;
	gpci.layout              = gfx->vk.pipeline_layout;
	gpci.renderPass          = s->rp->renderpass;
	gpci.subpass             = 0;
	gpci.basePipelineHandle  = NULL;
	gpci.basePipelineIndex   = -1;

	if (vkCreateGraphicsPipelines(gfx->vk.gpu, NULL, 1, &gpci, NULL, &s->pipeline) != VK_SUCCESS) {

		s->pipeline = NULL;
		return false;

	}

	gpu_uniform_shader ubs;

	ubs.screen = simd4f_create(
		s->viewport.width,
		s->viewport.height,
		s->viewport.width  / s->viewport.height,
		s->viewport.height / s->viewport.width);

	memcpy(&ubs.arg,      s->args,            sizeof(ubs.arg));
	memcpy(&ubs.textures, s->rp->surface_ids, sizeof(ubs.textures));

	gr_uniformbuffer_commit(&s->ub, &ubs, sizeof(ubs));

	return true;

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

	s->pipeline = NULL;

	mzero(s->module);
	mzero(s->entrypoints);
	szero(s->vibd);
	mzero(s->viad);
	mzero(s->pssci);
	szero(s->pvisci);
	szero(s->piasci);
	szero(s->pvsci);
	szero(s->prsci);
	szero(s->pmsci);
	szero(s->pdssci);
	mzero(s->pcbas);
	szero(s->pcbsci);

	mzero(s->args);

	gr_shader_defaults(s);

}



void destroy(gr_shader *s)
{

	for (int n=0; n < GR_SHADER_MAX; n++) {

		if (s->module[n] != NULL)
			vkDestroyShaderModule(gfx->vk.gpu, s->module[n], NULL);

		free((char*)s->entrypoints[n]);

	}

	if (s->pipeline != NULL)
		vkDestroyPipeline(gfx->vk.gpu, s->pipeline, NULL);

	reset(s);

}

