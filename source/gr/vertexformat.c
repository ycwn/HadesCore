

#include "core/system.h"
#include "core/types.h"
#include "core/common.h"
#include "core/debug.h"

#include "gr/limits.h"
#include "gr/vertexformat.h"


struct elements_formats {

	uint mask;
	uint value;
	uint components;
	uint binding;
	uint format;
	uint size;

} formats[] = {

	{ GR_V3, GR_V2, 2, GR_VERTEX, VK_FORMAT_R16G16_UNORM,       4 },
	{ GR_V3, GR_V3, 3, GR_VERTEX, VK_FORMAT_R16G16B16A16_UNORM, 8 },

	{ GR_T3,     0, 0, GR_TEXTURE, 0,                            0 },
	{ GR_T3, GR_T2, 2, GR_TEXTURE, VK_FORMAT_R16G16_UNORM,       4 },
	{ GR_T3, GR_T3, 3, GR_TEXTURE, VK_FORMAT_R16G16B16A16_SNORM, 8 },

	{ GR_C3,     0, 0, GR_COLOR, 0,                                 0 },
	{ GR_C3, GR_C3, 3, GR_COLOR, VK_FORMAT_B10G11R11_UFLOAT_PACK32, 4 },

	{ GR_Q4,     0, 0, GR_TANGENT, 0,                                  0 },
	{ GR_Q4, GR_Q4, 3, GR_TANGENT, VK_FORMAT_A2B10G10R10_SNORM_PACK32, 4 },

	{ GR_X4,     0, 0, GR_EXTRA, 0,                             0 },
	{ GR_X4, GR_X2, 2, GR_EXTRA, VK_FORMAT_R16G16_SFLOAT,       4 },
	{ GR_X4, GR_X4, 4, GR_EXTRA, VK_FORMAT_R16G16B16A16_SFLOAT, 8 },

	{ GR_B2,     0, 0, GR_BONE0, 0,                           0 },
	{ GR_B2, GR_B2, 4, GR_BONE0, VK_FORMAT_R16G16B16A16_UINT, 8 },

	{ GR_B4,     0, 0, GR_BONE1, 0,                           0 },
	{ GR_B4, GR_B4, 4, GR_BONE1, VK_FORMAT_R16G16B16A16_UINT, 8 },

	{ GR_B6,     0, 0, GR_BONE2, 0,                           0 },
	{ GR_B6, GR_B6, 4, GR_BONE2, VK_FORMAT_R16G16B16A16_UINT, 8 },

	{ GR_B8,     0, 0, GR_BONE3, 0,                           0 },
	{ GR_B8, GR_B8, 4, GR_BONE3, VK_FORMAT_R16G16B16A16_UINT, 8 }

};


static uint init_element(uint format, uint mask, uint offset, struct gr_vertexformat_element *vfe);
static uint init_descriptor(const struct gr_vertexformat_element *vfe, VkVertexInputAttributeDescription *desc);



void gr_vf_init(gr_vertexformat *vf, uint format)
{

	vf->format = format;
	vf->stride = 0;

	vf->stride += init_element(format, GR_V3, vf->stride, &vf->vertex);
	vf->stride += init_element(format, GR_T3, vf->stride, &vf->texture);
	vf->stride += init_element(format, GR_C3, vf->stride, &vf->color);
	vf->stride += init_element(format, GR_Q4, vf->stride, &vf->tangent);
	vf->stride += init_element(format, GR_X4, vf->stride, &vf->extra);
	vf->stride += init_element(format, GR_B2, vf->stride, &vf->bones[0]);
	vf->stride += init_element(format, GR_B4, vf->stride, &vf->bones[1]);
	vf->stride += init_element(format, GR_B6, vf->stride, &vf->bones[2]);
	vf->stride += init_element(format, GR_B8, vf->stride, &vf->bones[3]);

}



uint gr_vf_build_descriptors(gr_vertexformat *vf, VkVertexInputAttributeDescription vd[GR_VERTEXATTR_MAX])
{

	uint num = 0;

	num += init_descriptor(&vf->vertex,   &vd[num]);
	num += init_descriptor(&vf->texture,  &vd[num]);
	num += init_descriptor(&vf->color,    &vd[num]);
	num += init_descriptor(&vf->tangent,  &vd[num]);
	num += init_descriptor(&vf->extra,    &vd[num]);
	num += init_descriptor(&vf->bones[0], &vd[num]);
	num += init_descriptor(&vf->bones[1], &vd[num]);
	num += init_descriptor(&vf->bones[2], &vd[num]);
	num += init_descriptor(&vf->bones[3], &vd[num]);

	return num;

}



uint init_element(uint format, uint mask, uint offset, struct gr_vertexformat_element *vfe)
{

	vfe->components = 0;
	vfe->binding    = 0;
	vfe->format     = 0;
	vfe->offset     = offset;
	vfe->size       = 0;

	for (int n=0; n < countof(formats); n++)
		if ((formats[n].mask == mask) && ((format & mask) == formats[n].value)) {

			vfe->components = formats[n].components;
			vfe->binding    = formats[n].binding;
			vfe->format     = formats[n].format;
			vfe->size       = formats[n].size;
			break;

		}

	return vfe->size;

}



uint init_descriptor(const struct gr_vertexformat_element *vfe, VkVertexInputAttributeDescription *desc)
{

	if (vfe->components == 0)
		return 0;

	desc->binding  = 0;
	desc->location = vfe->binding;
	desc->format   = vfe->format;
	desc->offset   = vfe->offset;

	return 1;

}
