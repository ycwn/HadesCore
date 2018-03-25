

import hades.graphics_limits;
import hades.graphics_types;


enum {
	GR_V2  = 0x0000, GR_V3 = 0x0001,
	GR_T2  = 0x0002, GR_T3 = 0x0006,
	GR_C3  = 0x0008,
	GR_N3  = 0x0010,
	GR_X1  = 0x0040, GR_X2 = 0x00c0, GR_X3 = 0x01c0, GR_X4 = 0x03c0,
	GR_B2  = 0x0400, GR_B4 = 0x0c00, GR_B6 = 0x1c00, GR_B8 = 0x3c00
}

enum {
	GR_VERTEX, GR_TEXTURE, GR_COLOR, GR_NORMAL, GR_EXTRA, GR_BONE0, GR_BONE1, GR_BONE2, GR_BONE3
}


struct gr_vertexformat {

	struct vertexformat_element {

		uint components;
		uint binding;
		uint format;
		uint offset;
		uint size;

	}

	vertexformat_element    vertex;
	vertexformat_element    texture;
	vertexformat_element    color;
	vertexformat_element    normal;
	vertexformat_element    tangent;
	vertexformat_element    extra;
	vertexformat_element[4] bones;

	uint format;
	uint stride;

}


extern(C) {

	void gr_vf_init(             gr_vertexformat *vf, uint format);
	uint gr_vf_build_descriptors(gr_vertexformat *vf, VkVertexInputAttributeDescription[GR_VERTEXATTR_MAX] vd);

}

