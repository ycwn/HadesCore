

#ifndef __GR_VERTEXFORMAT_H
#define __GR_VERTEXFORMAT_H


enum {
	GR_V2  = 0x0000, GR_V3 = 0x0001,
	GR_T2  = 0x0002, GR_T3 = 0x0006,
	GR_C3  = 0x0008,
	GR_Q4  = 0x0010,
	GR_X2  = 0x0020, GR_X4 = 0x0060,
	GR_B2  = 0x0080, GR_B4 = 0x0180, GR_B6 = 0x0380, GR_B8 = 0x0780
};


enum {
	GR_VERTEX  = GPU_ATTRIBUTE_VERTEX,
	GR_TEXTURE = GPU_ATTRIBUTE_TEXTURE,
	GR_COLOR   = GPU_ATTRIBUTE_COLOR,
	GR_TANGENT = GPU_ATTRIBUTE_TANGENT,
	GR_EXTRA   = GPU_ATTRIBUTE_EXTRA,
	GR_BONE0   = GPU_ATTRIBUTE_BONE0,
	GR_BONE1   = GPU_ATTRIBUTE_BONE1,
	GR_BONE2   = GPU_ATTRIBUTE_BONE2,
	GR_BONE3   = GPU_ATTRIBUTE_BONE3
};


struct _gr_vertexformat_element_t {

	uint components;
	uint binding;
	uint format;
	uint offset;
	uint size;

};


typedef struct _gr_vertexformat_t {

	struct _gr_vertexformat_element_t vertex;
	struct _gr_vertexformat_element_t texture;
	struct _gr_vertexformat_element_t color;
	struct _gr_vertexformat_element_t tangent;
	struct _gr_vertexformat_element_t extra;
	struct _gr_vertexformat_element_t bones[4];

	uint format;
	uint stride;

} gr_vertexformat;


void gr_vf_init(             gr_vertexformat *vf, uint format);
uint gr_vf_build_descriptors(gr_vertexformat *vf, VkVertexInputAttributeDescription *vd);


#endif

