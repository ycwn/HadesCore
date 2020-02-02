

#include "core/system.h"
#include "core/types.h"
#include "core/common.h"
#include "core/debug.h"
#include "core/blob.h"
#include "core/list.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/pixelformat.h"
#include "gr/dds.h"


static const uint DDS_MAGIC = 0x20534444;

static const uint DDS_CAPS    = 1 << 0;
static const uint DDS_WIDTH   = 1 << 1;
static const uint DDS_HEIGHT  = 1 << 2;
static const uint DDS_PITCH   = 1 << 3;
static const uint DDS_PF      = 1 << 12;
static const uint DDS_MIPMAPS = 1 << 17;
static const uint DDS_LINEAR  = 1 << 19;
static const uint DDS_DEPTH   = 1 << 23;
static const uint DDS_TEXTURE = DDS_CAPS | DDS_WIDTH | DDS_HEIGHT | DDS_PF;

static const uint DDS_ALPHAPIXELS = 1 << 0;
static const uint DDS_ALPHA       = 1 << 1;
static const uint DDS_FOURCC      = 1 << 2;
static const uint DDS_INDEXED     = 1 << 5;
static const uint DDS_RGB         = 1 << 6;
static const uint DDS_COMPRESSED  = 1 << 7;
static const uint DDS_LUMINANCE   = 1 << 17;

static const uint DDS_SURFACE_COMPLEX = 1 << 3;
static const uint DDS_SURFACE_TEXTURE = 1 << 12;
static const uint DDS_SURFACE_MIPMAP  = 1 << 22;

static const uint DDS_CUBEMAP       = 1 << 9;
static const uint DDS_CUBEMAP_POS_X = 1 << 10;
static const uint DDS_CUBEMAP_NEG_X = 1 << 11;
static const uint DDS_CUBEMAP_POS_Y = 1 << 12;
static const uint DDS_CUBEMAP_NEG_Y = 1 << 13;
static const uint DDS_CUBEMAP_POS_Z = 1 << 14;
static const uint DDS_CUBEMAP_NEG_Z = 1 << 15;
static const uint DDS_VOLUME        = 1 << 21;
static const uint DDS_CUBEMAP_FULL  =
		DDS_CUBEMAP_POS_X | DDS_CUBEMAP_NEG_X | DDS_CUBEMAP_POS_Y |
		DDS_CUBEMAP_NEG_Y | DDS_CUBEMAP_POS_Z | DDS_CUBEMAP_NEG_Z;

static const uint D3DFMT_A16B16G16R16  =  36;
static const uint D3DFMT_Q16W16V16U16  = 110;
static const uint D3DFMT_R16F          = 111;
static const uint D3DFMT_G16R16F       = 112;
static const uint D3DFMT_A16B16G16R16F = 113;
static const uint D3DFMT_R32F          = 114;
static const uint D3DFMT_G32R32F       = 115;
static const uint D3DFMT_A32B32G32R32F = 116;

static const uint D3DFMT_DX10 = 0x30315844; // DX10
static const uint D3DFMT_RGBG = 0x47424752; // RGBG
static const uint D3DFMT_YUY2 = 0x32595559; // YUY2
static const uint D3DFMT_GRGB = 0x42475247; // GRGB
static const uint D3DFMT_DXT1 = 0x31545844; // DXT1
static const uint D3DFMT_DXT2 = 0x32545844; // DXT2
static const uint D3DFMT_DXT3 = 0x33545844; // DXT3
static const uint D3DFMT_DXT4 = 0x34545844; // DXT4
static const uint D3DFMT_DXT5 = 0x35545844; // DXT5
static const uint D3DFMT_ATI1 = 0x31495441; // ATI1
static const uint D3DFMT_ATI2 = 0x32495441; // ATI2
static const uint D3DFMT_BC4U = 0x55344342; // BC4U
static const uint D3DFMT_BC4S = 0x53344342; // BC4S
static const uint D3DFMT_BC5U = 0x55354342; // BC5U
static const uint D3DFMT_BC5S = 0x53354342; // BC5S

static const uint DXGI_FORMAT_UNKNOWN                    = 0;
static const uint DXGI_FORMAT_R32G32B32A32_FLOAT         = 2;
static const uint DXGI_FORMAT_R32G32B32A32_UINT          = 3;
static const uint DXGI_FORMAT_R32G32B32A32_SINT          = 4;
static const uint DXGI_FORMAT_R32G32B32_FLOAT            = 6;
static const uint DXGI_FORMAT_R32G32B32_UINT             = 7;
static const uint DXGI_FORMAT_R32G32B32_SINT             = 8;
static const uint DXGI_FORMAT_R16G16B16A16_FLOAT         = 10;
static const uint DXGI_FORMAT_R16G16B16A16_UNORM         = 11;
static const uint DXGI_FORMAT_R16G16B16A16_UINT          = 12;
static const uint DXGI_FORMAT_R16G16B16A16_SNORM         = 13;
static const uint DXGI_FORMAT_R16G16B16A16_SINT          = 14;
static const uint DXGI_FORMAT_R32G32_FLOAT               = 16;
static const uint DXGI_FORMAT_R32G32_UINT                = 17;
static const uint DXGI_FORMAT_R32G32_SINT                = 18;
static const uint DXGI_FORMAT_D32_FLOAT_S8X24_UINT       = 20;
static const uint DXGI_FORMAT_R10G10B10A2_UNORM          = 24;
static const uint DXGI_FORMAT_R10G10B10A2_UINT           = 25;
static const uint DXGI_FORMAT_R11G11B10_FLOAT            = 26;
static const uint DXGI_FORMAT_R8G8B8A8_UNORM             = 28;
static const uint DXGI_FORMAT_R8G8B8A8_UNORM_SRGB        = 29;
static const uint DXGI_FORMAT_R8G8B8A8_UINT              = 30;
static const uint DXGI_FORMAT_R8G8B8A8_SNORM             = 31;
static const uint DXGI_FORMAT_R8G8B8A8_SINT              = 32;
static const uint DXGI_FORMAT_R16G16_FLOAT               = 34;
static const uint DXGI_FORMAT_R16G16_UNORM               = 35;
static const uint DXGI_FORMAT_R16G16_UINT                = 36;
static const uint DXGI_FORMAT_R16G16_SNORM               = 37;
static const uint DXGI_FORMAT_R16G16_SINT                = 38;
static const uint DXGI_FORMAT_D32_FLOAT                  = 40;
static const uint DXGI_FORMAT_R32_FLOAT                  = 41;
static const uint DXGI_FORMAT_R32_UINT                   = 42;
static const uint DXGI_FORMAT_R32_SINT                   = 43;
static const uint DXGI_FORMAT_D24_UNORM_S8_UINT          = 45;
static const uint DXGI_FORMAT_R8G8_UNORM                 = 49;
static const uint DXGI_FORMAT_R8G8_UINT                  = 50;
static const uint DXGI_FORMAT_R8G8_SNORM                 = 51;
static const uint DXGI_FORMAT_R8G8_SINT                  = 52;
static const uint DXGI_FORMAT_R16_FLOAT                  = 54;
static const uint DXGI_FORMAT_D16_UNORM                  = 55;
static const uint DXGI_FORMAT_R16_UNORM                  = 56;
static const uint DXGI_FORMAT_R16_UINT                   = 57;
static const uint DXGI_FORMAT_R16_SNORM                  = 58;
static const uint DXGI_FORMAT_R16_SINT                   = 59;
static const uint DXGI_FORMAT_R8_UNORM                   = 61;
static const uint DXGI_FORMAT_R8_UINT                    = 62;
static const uint DXGI_FORMAT_R8_SNORM                   = 63;
static const uint DXGI_FORMAT_R8_SINT                    = 64;
static const uint DXGI_FORMAT_A8_UNORM                   = 65;
static const uint DXGI_FORMAT_R1_UNORM                   = 66;
static const uint DXGI_FORMAT_R9G9B9E5_SHAREDEXP         = 67;
static const uint DXGI_FORMAT_BC1_UNORM                  = 71;
static const uint DXGI_FORMAT_BC1_UNORM_SRGB             = 72;
static const uint DXGI_FORMAT_BC2_UNORM                  = 74;
static const uint DXGI_FORMAT_BC2_UNORM_SRGB             = 75;
static const uint DXGI_FORMAT_BC3_UNORM                  = 77;
static const uint DXGI_FORMAT_BC3_UNORM_SRGB             = 78;
static const uint DXGI_FORMAT_BC4_UNORM                  = 80;
static const uint DXGI_FORMAT_BC4_SNORM                  = 81;
static const uint DXGI_FORMAT_BC5_UNORM                  = 83;
static const uint DXGI_FORMAT_BC5_SNORM                  = 84;
static const uint DXGI_FORMAT_B5G6R5_UNORM               = 85;
static const uint DXGI_FORMAT_B5G5R5A1_UNORM             = 86;
static const uint DXGI_FORMAT_B8G8R8A8_UNORM             = 87;
static const uint DXGI_FORMAT_B8G8R8X8_UNORM             = 88;
static const uint DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89;
static const uint DXGI_FORMAT_B8G8R8A8_UNORM_SRGB        = 91;
static const uint DXGI_FORMAT_B8G8R8X8_UNORM_SRGB        = 93;
static const uint DXGI_FORMAT_BC6H_UF16                  = 95;
static const uint DXGI_FORMAT_BC6H_SF16                  = 96;
static const uint DXGI_FORMAT_BC7_UNORM                  = 98;
static const uint DXGI_FORMAT_BC7_UNORM_SRGB             = 99;
static const uint DXGI_FORMAT_B4G4R4A4_UNORM             = 115;


struct dds_header {

	u32 size;
	u32 flags;

	u32 height;
	u32 width;
	u32 pitch;
	u32 depth;

	u32 mipmaps;

	u32 _reserved0[11];

	struct {

		u32 size;
		u32 flags;
		u32 fourcc;

		u32 bits;

		u32 R_mask;
		u32 G_mask;
		u32 B_mask;
		u32 A_mask;

	} pf;

	u32 caps0;
	u32 caps1;
	u32 caps2;
	u32 caps3;

	u32 _reserved1;

	u8 data[0];

};

struct dds_header10 {

	u32 format;
	u32 dimension;

	u32 flags;
	u32 count;
	u32 reserved;

	u8 data[0];

};

struct dds {

	u32 magic;

	struct dds_header   header;
	struct dds_header10 header10;

};


static uint find_pixelformat(const struct dds *dx);
static uint calculate_image_length( const gr_dds *dds);
static uint calculate_mipmap_length(const gr_pixelformat *pf, uint width, uint height, uint depth);


#define IS_MASK(R, G, B, A) \
		(dx->header.pf.R_mask == (R) && dx->header.pf.G_mask == (G) && \
		 dx->header.pf.B_mask == (B) && dx->header.pf.A_mask == (A))

#define IS_FOURCC(FOURCC) \
		(dx->header.pf.fourcc == (FOURCC))



void gr_dds_init(gr_dds *dds)
{

	dds->format = gr_pixelformat_get(VK_FORMAT_UNDEFINED);
	dds->data   = NULL;

	dds->image_count  = 0;
	dds->mipmap_count = 0;
	dds->is_cubemap   = false;

	dds->image.pixels = NULL;
	dds->image.width  = 0;
	dds->image.height = 0;
	dds->image.depth  = 0;
	dds->image.length = 0;
	dds->image.offset = 0;

	dds->mipmap.pixels = NULL;
	dds->mipmap.width  = 0;
	dds->mipmap.height = 0;
	dds->mipmap.depth  = 0;
	dds->mipmap.length = 0;
	dds->image.offset = 0;

}



bool gr_dds_open(gr_dds *dds, const char *file)
{

	return gr_dds_openfd(dds, blob_open(file, BLOB_REV_LAST));
}



bool gr_dds_openfd(gr_dds *dds, int id)
{

	return gr_dds_load(dds, blob_get_data(id));

}



bool gr_dds_load(gr_dds *dds, const void *ptr)
{

	const struct dds *dx   = ptr;
	bool              dx10 = false;

	gr_dds_init(dds);

	if (dx == NULL || dx->magic != DDS_MAGIC)
		return false;

	if (dx->header.size != sizeof(struct dds_header) || (dx->header.flags & DDS_TEXTURE) != DDS_TEXTURE)
		return false;

	if (dx->header.caps1 & DDS_CUBEMAP) {

		if ((dx->header.caps1 & DDS_CUBEMAP_FULL) != DDS_CUBEMAP_FULL)
			return false;

		dds->is_cubemap = true;

	}

	if (dx->header.pf.fourcc == D3DFMT_DX10) {

		if (dx->header10.count < 1)
			return false;

		dx10 = true;

	}

	dds->format       = gr_pixelformat_get(find_pixelformat(dx));
	dds->data         = dx10? &dx->header10.data[0]: &dx->header.data[0];
	dds->image_count  = dx10? dx->header10.count: 1;
	dds->mipmap_count = (dx->header.flags & DDS_MIPMAPS)? dx->header.mipmaps: 1;

	dds->image.pixels = dds->data;
	dds->image.width  = dx->header.width;
	dds->image.height = dx->header.height;
	dds->image.depth  = (dx->header.flags & DDS_DEPTH)? dx->header.depth: 1;
	dds->image.length = calculate_image_length(dds);

	if (!gr_dds_is_valid(dds))
		return false;

	gr_dds_select(dds, 0, 0);
	return true;

}



bool gr_dds_select(gr_dds *dds, uint image, uint mipmap)
{

	if (image >= dds->image_count || mipmap >= dds->mipmap_count)
		return false;

	dds->image.offset = dds->image.length * image;
	dds->image.pixels = dds->data + dds->image.offset;

	dds->mipmap.offset = dds->image.offset;
	dds->mipmap.pixels = dds->image.pixels;

	dds->mipmap.width  = dds->image.width;
	dds->mipmap.height = dds->image.height;
	dds->mipmap.depth  = dds->image.depth;
	dds->mipmap.length = calculate_mipmap_length(dds->format, dds->image.width, dds->image.height, dds->image.depth);

	for (int n=0; n < mipmap; n++) {

		dds->mipmap.offset += dds->mipmap.length;
		dds->mipmap.pixels += dds->mipmap.length;

		dds->mipmap.width  = maxu(dds->mipmap.width  >> 1, 1);
		dds->mipmap.height = maxu(dds->mipmap.height >> 1, 1);
		dds->mipmap.depth  = maxu(dds->mipmap.depth  >> 1, 1);
		dds->mipmap.length = calculate_mipmap_length(dds->format, dds->mipmap.width, dds->mipmap.height, dds->mipmap.depth);

	}

	return true;

}



uint find_pixelformat(const struct dds *dx)
{

	if (dx->header.pf.flags & DDS_RGB) {

		if (dx->header.pf.bits == 32)
			return
				IS_MASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000)? VK_FORMAT_R8G8B8A8_UNORM: //FIXME:Endian?
				IS_MASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000)? VK_FORMAT_B8G8R8A8_UNORM: //FIXME:Endian?
				IS_MASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000)? VK_FORMAT_B8G8R8A8_UNORM: //FIXME:Endian?
				IS_MASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000)? VK_FORMAT_A2B10G10R10_UNORM_PACK32://FIXME:Endian?
				IS_MASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000)? VK_FORMAT_R16G16_UNORM:
				IS_MASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000)? VK_FORMAT_R32_SFLOAT:
				VK_FORMAT_UNDEFINED;

		if (dx->header.pf.bits == 16)
			return
				IS_MASK(0x7c00, 0x03e0, 0x001f, 0x8000)? VK_FORMAT_B5G5R5A1_UNORM_PACK16:
				IS_MASK(0xf800, 0x07e0, 0x001f, 0x0000)? VK_FORMAT_B5G6R5_UNORM_PACK16:
				IS_MASK(0x0f00, 0x00f0, 0x000f, 0xf000)? VK_FORMAT_B4G4R4A4_UNORM_PACK16:
				VK_FORMAT_UNDEFINED;

		return VK_FORMAT_UNDEFINED;

	}

	if (dx->header.pf.flags & DDS_LUMINANCE) {

		if (dx->header.pf.bits == 16)
			return
				IS_MASK(0xffff, 0x0000, 0x0000, 0x0000)? VK_FORMAT_R16_UNORM:
				IS_MASK(0x00ff, 0x0000, 0x0000, 0xff00)? VK_FORMAT_R8G8_UNORM:
				VK_FORMAT_UNDEFINED;

		if (dx->header.pf.bits == 8)
			return
				IS_MASK(0xff, 0x00, 0x00, 0x00)? VK_FORMAT_R8_UNORM:
				VK_FORMAT_UNDEFINED;

		return VK_FORMAT_UNDEFINED;

	}

	if (dx->header.pf.flags & DDS_ALPHA) {

		if (dx->header.pf.bits == 8)
			return VK_FORMAT_R8_UNORM;

		return VK_FORMAT_UNDEFINED;

	}

	if (dx->header.pf.flags & DDS_FOURCC) {

		if (!IS_FOURCC(D3DFMT_DX10))
			return
				IS_FOURCC(D3DFMT_DXT1)?          VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
				IS_FOURCC(D3DFMT_DXT2)?          VK_FORMAT_BC2_UNORM_BLOCK:
				IS_FOURCC(D3DFMT_DXT3)?          VK_FORMAT_BC2_UNORM_BLOCK:
				IS_FOURCC(D3DFMT_DXT4)?          VK_FORMAT_BC3_UNORM_BLOCK:
				IS_FOURCC(D3DFMT_DXT5)?          VK_FORMAT_BC3_UNORM_BLOCK:
				IS_FOURCC(D3DFMT_ATI1)?          VK_FORMAT_BC4_UNORM_BLOCK:
				IS_FOURCC(D3DFMT_BC4U)?          VK_FORMAT_BC4_UNORM_BLOCK:
				IS_FOURCC(D3DFMT_BC4S)?          VK_FORMAT_BC4_SNORM_BLOCK:
				IS_FOURCC(D3DFMT_ATI2)?          VK_FORMAT_BC5_UNORM_BLOCK:
				IS_FOURCC(D3DFMT_BC5U)?          VK_FORMAT_BC5_UNORM_BLOCK:
				IS_FOURCC(D3DFMT_BC5S)?          VK_FORMAT_BC5_SNORM_BLOCK:
				IS_FOURCC(D3DFMT_A16B16G16R16)?  VK_FORMAT_R16G16B16A16_UNORM:
				IS_FOURCC(D3DFMT_Q16W16V16U16)?  VK_FORMAT_R16G16B16A16_SNORM:
				IS_FOURCC(D3DFMT_R16F)?          VK_FORMAT_R16_SFLOAT:
				IS_FOURCC(D3DFMT_G16R16F)?       VK_FORMAT_R16G16_SFLOAT:
				IS_FOURCC(D3DFMT_A16B16G16R16F)? VK_FORMAT_R16G16B16A16_SFLOAT:
				IS_FOURCC(D3DFMT_R32F)?          VK_FORMAT_R32_SFLOAT:
				IS_FOURCC(D3DFMT_G32R32F)?       VK_FORMAT_R32G32_SFLOAT:
				IS_FOURCC(D3DFMT_A32B32G32R32F)? VK_FORMAT_R32G32B32A32_SFLOAT:
				VK_FORMAT_UNDEFINED;

		switch (dx->header10.format) {

			case DXGI_FORMAT_R32G32B32A32_FLOAT:   return VK_FORMAT_R32G32B32A32_SFLOAT;
			case DXGI_FORMAT_R32G32B32A32_UINT:    return VK_FORMAT_R32G32B32A32_UINT;
			case DXGI_FORMAT_R32G32B32A32_SINT:    return VK_FORMAT_R32G32B32A32_SINT;
			case DXGI_FORMAT_R32G32B32_FLOAT:      return VK_FORMAT_R32G32B32_SFLOAT;
			case DXGI_FORMAT_R32G32B32_UINT:       return VK_FORMAT_R32G32B32_UINT;
			case DXGI_FORMAT_R32G32B32_SINT:       return VK_FORMAT_R32G32B32_SINT;
			case DXGI_FORMAT_R16G16B16A16_FLOAT:   return VK_FORMAT_R16G16B16A16_SFLOAT;
			case DXGI_FORMAT_R16G16B16A16_UNORM:   return VK_FORMAT_R16G16B16A16_UNORM;
			case DXGI_FORMAT_R16G16B16A16_UINT:    return VK_FORMAT_R16G16B16A16_UINT;
			case DXGI_FORMAT_R16G16B16A16_SNORM:   return VK_FORMAT_R16G16B16A16_SNORM;
			case DXGI_FORMAT_R16G16B16A16_SINT:    return VK_FORMAT_R16G16B16A16_SINT;
			case DXGI_FORMAT_R32G32_FLOAT:         return VK_FORMAT_R32G32_SFLOAT;
			case DXGI_FORMAT_R32G32_UINT:          return VK_FORMAT_R32G32_UINT;
			case DXGI_FORMAT_R32G32_SINT:          return VK_FORMAT_R32G32_SINT;
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
			case DXGI_FORMAT_R10G10B10A2_UNORM:    return VK_FORMAT_A2B10G10R10_UNORM_PACK32; //FIXME:Endian?
			case DXGI_FORMAT_R10G10B10A2_UINT:     return VK_FORMAT_A2B10G10R10_UINT_PACK32;  //FIXME:Endian?
			case DXGI_FORMAT_R11G11B10_FLOAT:      return VK_FORMAT_B10G11R11_UFLOAT_PACK32;  //FIXME:Endian?
			case DXGI_FORMAT_R8G8B8A8_UNORM:       return VK_FORMAT_R8G8B8A8_UNORM;
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:  return VK_FORMAT_R8G8B8A8_SRGB;
			case DXGI_FORMAT_R8G8B8A8_UINT:        return VK_FORMAT_R8G8B8A8_UINT;
			case DXGI_FORMAT_R8G8B8A8_SNORM:       return VK_FORMAT_R8G8B8A8_SNORM;
			case DXGI_FORMAT_R8G8B8A8_SINT:        return VK_FORMAT_R8G8B8A8_SINT;
			case DXGI_FORMAT_R16G16_FLOAT:         return VK_FORMAT_R16G16_SFLOAT;
			case DXGI_FORMAT_R16G16_UNORM:         return VK_FORMAT_R16G16_UNORM;
			case DXGI_FORMAT_R16G16_UINT:          return VK_FORMAT_R16G16_UINT;
			case DXGI_FORMAT_R16G16_SNORM:         return VK_FORMAT_R16G16_SNORM;
			case DXGI_FORMAT_R16G16_SINT:          return VK_FORMAT_R16G16_SINT;
			case DXGI_FORMAT_D32_FLOAT:            return VK_FORMAT_D32_SFLOAT;
			case DXGI_FORMAT_R32_FLOAT:            return VK_FORMAT_R32_SFLOAT;
			case DXGI_FORMAT_R32_UINT:             return VK_FORMAT_R32_UINT;
			case DXGI_FORMAT_R32_SINT:             return VK_FORMAT_R32_SINT;
			case DXGI_FORMAT_D24_UNORM_S8_UINT:    return VK_FORMAT_D24_UNORM_S8_UINT;
			case DXGI_FORMAT_R8G8_UNORM:           return VK_FORMAT_R8G8_UNORM;
			case DXGI_FORMAT_R8G8_UINT:            return VK_FORMAT_R8G8_UINT;
			case DXGI_FORMAT_R8G8_SNORM:           return VK_FORMAT_R8G8_SNORM;
			case DXGI_FORMAT_R8G8_SINT:            return VK_FORMAT_R8G8_SINT;
			case DXGI_FORMAT_R16_FLOAT:            return VK_FORMAT_R16_SFLOAT;
			case DXGI_FORMAT_D16_UNORM:            return VK_FORMAT_D16_UNORM;
			case DXGI_FORMAT_R16_UNORM:            return VK_FORMAT_R16_UNORM;
			case DXGI_FORMAT_R16_UINT:             return VK_FORMAT_R16_UINT;
			case DXGI_FORMAT_R16_SNORM:            return VK_FORMAT_R16_SNORM;
			case DXGI_FORMAT_R16_SINT:             return VK_FORMAT_R16_SINT;
			case DXGI_FORMAT_R8_UNORM:             return VK_FORMAT_R8_UNORM;
			case DXGI_FORMAT_R8_UINT:              return VK_FORMAT_R8_UINT;
			case DXGI_FORMAT_R8_SNORM:             return VK_FORMAT_R8_SNORM;
			case DXGI_FORMAT_R8_SINT:              return VK_FORMAT_R8_SINT;
			case DXGI_FORMAT_A8_UNORM:             return VK_FORMAT_R8_UNORM;
			case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:   return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32; //FIXME:Endian?
			case DXGI_FORMAT_BC1_UNORM:            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
			case DXGI_FORMAT_BC1_UNORM_SRGB:       return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
			case DXGI_FORMAT_BC2_UNORM:            return VK_FORMAT_BC2_UNORM_BLOCK;
			case DXGI_FORMAT_BC2_UNORM_SRGB:       return VK_FORMAT_BC2_SRGB_BLOCK;
			case DXGI_FORMAT_BC3_UNORM:            return VK_FORMAT_BC3_UNORM_BLOCK;
			case DXGI_FORMAT_BC3_UNORM_SRGB:       return VK_FORMAT_BC3_SRGB_BLOCK;
			case DXGI_FORMAT_BC4_UNORM:            return VK_FORMAT_BC4_UNORM_BLOCK;
			case DXGI_FORMAT_BC4_SNORM:            return VK_FORMAT_BC4_SNORM_BLOCK;
			case DXGI_FORMAT_BC5_UNORM:            return VK_FORMAT_BC5_UNORM_BLOCK;
			case DXGI_FORMAT_BC5_SNORM:            return VK_FORMAT_BC5_SNORM_BLOCK;
			case DXGI_FORMAT_B5G6R5_UNORM:         return VK_FORMAT_B5G6R5_UNORM_PACK16;
			case DXGI_FORMAT_B5G5R5A1_UNORM:       return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
			case DXGI_FORMAT_B8G8R8A8_UNORM:       return VK_FORMAT_B8G8R8A8_UNORM;
			case DXGI_FORMAT_B8G8R8X8_UNORM:       return VK_FORMAT_B8G8R8A8_UNORM;
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:  return VK_FORMAT_B8G8R8A8_SRGB;
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:  return VK_FORMAT_B8G8R8A8_SRGB;
			case DXGI_FORMAT_BC6H_UF16:            return VK_FORMAT_BC6H_UFLOAT_BLOCK;
			case DXGI_FORMAT_BC6H_SF16:            return VK_FORMAT_BC6H_SFLOAT_BLOCK;
			case DXGI_FORMAT_BC7_UNORM:            return VK_FORMAT_BC7_UNORM_BLOCK;
			case DXGI_FORMAT_BC7_UNORM_SRGB:       return VK_FORMAT_BC7_SRGB_BLOCK;
			case DXGI_FORMAT_B4G4R4A4_UNORM:       return VK_FORMAT_B4G4R4A4_UNORM_PACK16;

		}

	}

	return VK_FORMAT_UNDEFINED;

}



uint calculate_image_length(const gr_dds *dds)
{

	uint len = 0;
	uint w = dds->image.width;
	uint h = dds->image.height;
	uint d = dds->image.depth;

	for (int n=0; n < dds->mipmap_count; n++) {

		len += calculate_mipmap_length(dds->format, w, h, d);

		w = maxu(w >> 1, 1);
		h = maxu(h >> 1, 1);
		d = maxu(d >> 1, 1);

	}

	return len;

}



uint calculate_mipmap_length(const gr_pixelformat *pf, uint width, uint height, uint depth)
{

	switch (pf->packing) {

		case GR_PIXELFORMAT_PACKING_LINEAR:
			return ((pf->bits + 7) / 8) * width * height * depth;

		case GR_PIXELFORMAT_PACKING_BLOCK: {
			const uint bx = maxu((width  + pf->block_w - 1) / pf->block_w, 1);
			const uint by = maxu((height + pf->block_h - 1) / pf->block_h, 1);
			const uint bz = maxu((depth  + pf->block_d - 1) / pf->block_d, 1);
			return pf->alignment * bx * by * bz;
		}

		case GR_PIXELFORMAT_PACKING_PACKED:
			return pf->alignment * ((width + 1) / 2) * height * depth;

	}

	return 0;

}
