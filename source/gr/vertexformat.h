

#ifndef GR_VERTEXFORMAT_H
#define GR_VERTEXFORMAT_H


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


struct gr_vertexformat_element {

	uint components;
	uint binding;
	uint format;
	uint offset;
	uint size;

};


typedef struct gr_vertexformat {

	struct gr_vertexformat_element vertex;
	struct gr_vertexformat_element texture;
	struct gr_vertexformat_element color;
	struct gr_vertexformat_element tangent;
	struct gr_vertexformat_element extra;
	struct gr_vertexformat_element bones[4];

	uint format;
	uint stride;

} gr_vertexformat;


void gr_vf_init(             gr_vertexformat *vf, uint format);
uint gr_vf_build_descriptors(gr_vertexformat *vf, VkVertexInputAttributeDescription vd[GR_VERTEXATTR_MAX]);


#endif

