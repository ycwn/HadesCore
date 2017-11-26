

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/blob.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/rendertarget.h"
#include "gr/vertexformat.h"
#include "gr/vertexbuffer.h"
#include "gr/uniformbuffer.h"
#include "gr/shader.h"
#include "gr/command.h"
#include "gr/commandqueue.h"
#include "gr/commandlist.h"
#include "gr/geometry.h"



static const uint GEOMETRY_MAGIC = 0x006f6567;  // 'geo\0'

static const uint CHUNK_DRAWCALL = 0x77617264;  // 'draw'
static const uint CHUNK_FORMI    = 0x69746d66;  // 'fmti'
static const uint CHUNK_FORMV    = 0x76746d66;  // 'fmtv'
static const uint CHUNK_INDICES  = 0x78646e69;  // 'indx'
static const uint CHUNK_STRINGS  = 0x73727473;  // 'strs'
static const uint CHUNK_TEXT     = 0x74786574;  // 'text'
static const uint CHUNK_VERTICES = 0x78747276;  // 'vrtx'

static const uint VF_V2 = GR_V2;  // 2D Vertex coordinates
static const uint VF_V3 = GR_V3;  // 3D Vertex coordinates
static const uint VF_T2 = GR_T2;  // 2D Texture coordinates
static const uint VF_T3 = GR_T3;  // 3D Texture coordinates
static const uint VF_C3 = GR_C3;  // RGB vertex colors
static const uint VF_Q4 = GR_Q4;  // 4D tangent space quaternion
static const uint VF_X2 = GR_X2;  // Free parameters 0 and 1
static const uint VF_X4 = GR_X4;  // Free parameters 2 and 3
static const uint VF_B2 = GR_B2;  // Bone + Weight for bones 0 and 1
static const uint VF_B4 = GR_B4;  // Bone + Weight for bones 2 and 3
static const uint VF_B6 = GR_B6;  // Bone + Weight for bones 4 and 5
static const uint VF_B8 = GR_B8;  // Bone + Weight for bones 6 and 7

static const uint IF_USHORT = 2;  // Indices are unsigned shorts
static const uint IF_UINT   = 4;  // Indices are unsigned ints



struct geo_chunk {

	u32 magic;
	u32 offset;
	u32 length;
	u32 padding;

};

typedef struct geo_header {

	u32 magic;
	u32 chunks;

	struct geo_chunk chunk[0];

} geo_header;



struct geo_drawcall {

	u32 name;
	u32 shader;

	u32 start;
	u32 count;

	f32 minimum[4];
	f32 maximum[4];

	f32 albedo[4];
	f32 emission[4];

	f32 transparency;
	f32 metallicity;
	f32 roughness;
	f32 refraction;

	f32 args[20];

};



struct geo_text {

	u32 tag;
	u32 text;

};



void gr_geometry_init(gr_geometry *geo)
{

	gr_vf_init(&geo->vf, VF_V2);

	geo->index_size = 0;

	geo->vertices       = NULL;
	geo->vertices_count = 0;
	geo->vertices_size  = 0;

	geo->indices       = NULL;
	geo->indices_count = 0;
	geo->indices_size  = 0;

	geo->drawcalls       = NULL;
	geo->drawcalls_count = 0;

	geo->strings      = "";
	geo->strings_size = 0;

}



bool gr_geometry_open(gr_geometry *geo, const char *file)
{

	return gr_geometry_openfd(geo, blob_open(file, BLOB_REV_LAST));

}



bool gr_geometry_openfd(gr_geometry *geo, int id)
{

	return gr_geometry_load(geo, blob_get_data(id));

}



bool gr_geometry_load(gr_geometry *geo, const void *ptr)
{

	if (geo == NULL || ptr == NULL)
		return false;

	const geo_header *header = ptr;

	if (header->magic != GEOMETRY_MAGIC)
		return false;

	gr_geometry_init(geo);

	uint vform = VF_V2;

	for (int n=0; n < header->chunks; n++)
		switch (header->chunk[n].magic) {

			case CHUNK_DRAWCALL:
				geo->drawcalls = (const u8*)ptr + header->chunk[n].offset;
				geo->drawcalls_count = header->chunk[n].length / sizeof(struct geo_drawcall);
				break;

			case CHUNK_FORMI:
				geo->index_size = *(const u32*)((const u8*)ptr + header->chunk[n].offset);
				break;

			case CHUNK_FORMV:
				vform = *(const u32*)((const u8*)ptr + header->chunk[n].offset);
				break;

			case CHUNK_INDICES:
				geo->indices      = (const u8*)ptr + header->chunk[n].offset;
				geo->indices_size = header->chunk[n].length;
				break;

			case CHUNK_STRINGS:
				geo->strings      = (const char*)ptr + header->chunk[n].offset;
				geo->strings_size = header->chunk[n].length;
				break;

			case CHUNK_VERTICES:
				geo->vertices      = (const u8*)ptr + header->chunk[n].offset;
				geo->vertices_size = header->chunk[n].length;
				break;

			default:
				log_e("geometry: warning: unknown chunk %#x", header->chunk[n].magic);
				break;

		}

	gr_vf_init(&geo->vf, vform);

	geo->indices_count  = (geo->index_size > 0)? geo->indices_size / geo->index_size: 0;
	geo->vertices_count = geo->vertices_size / geo->vf.stride;

	return true;

}



void gr_geometry_commit_drawcalls(const gr_geometry *geo, gr_commandlist *cmdl)
{

	for (int n=0; n < geo->drawcalls_count; n++) {

		const struct geo_drawcall *dc = (const struct geo_drawcall*)geo->drawcalls + n;

		gr_shader *shader = gr_shader_find(gr_geometry_get_text(geo, dc->shader));

		if (shader == NULL) {

			log_e("geometry: warning: unknown shader '%s'", gr_geometry_get_text(geo, dc->shader));
			continue;

		}

		gr_command *cmd = gr_commandlist_append(cmdl, shader, 1, true);

		cmd->start = dc->start;
		cmd->count = dc->count;

	}

}



void gr_geometry_commit_buffers(const gr_geometry *geo, gr_vertexbuffer *vb)
{

	gr_vertexbuffer_commit_vertices(vb, geo->vertices, geo->vertices_size);

	if (geo->indices_size > 0)
		gr_vertexbuffer_commit_indices(vb, geo->indices, geo->indices_size);

	else
		gr_vertexbuffer_commit_indices(vb, NULL, 0);

}



void gr_geometry_commit_uniforms(const gr_geometry *geo, gpu_uniform_material *ub, uint n)
{

	if (n > geo->drawcalls_count)
		return;

	const struct geo_drawcall *dc = (const struct geo_drawcall*)geo->drawcalls + n;

	ub->minimum = simd4f_uload4(dc->minimum);
	ub->maximum = simd4f_uload4(dc->maximum);

	ub->albedo   = simd4f_uload4(dc->albedo);
	ub->emission = simd4f_uload4(dc->emission);
	ub->material = simd4f_create(dc->transparency, dc->metallicity, dc->roughness, dc->refraction);

	for (int m=0; m < 5; m++)
		ub->arg[m] = simd4f_uload4(&dc->args[m * 4]);

}



const gr_material *gr_geometry_get_material(const gr_geometry *geo, gr_material *tmp, uint n)
{

	if (n > geo->drawcalls_count)
		return NULL;

	const struct geo_drawcall *dc = (const struct geo_drawcall*)geo->drawcalls + n;

	tmp->name   = gr_geometry_get_text(geo, dc->name);
	tmp->shader = gr_geometry_get_text(geo, dc->shader);

	tmp->minimum = simd4f_uload4(dc->minimum);
	tmp->maximum = simd4f_uload4(dc->maximum);

	tmp->albedo   = simd4f_uload4(dc->albedo);
	tmp->emission = simd4f_uload4(dc->emission);

	tmp->transparency = dc->transparency;
	tmp->metallicity  = dc->metallicity;
	tmp->roughness    = dc->roughness;
	tmp->refraction   = dc->refraction;

	for (int m=0; m < 5; m++)
		tmp->args[m] = simd4f_uload4(&dc->args[m * 4]);

	return tmp;

}



const char *gr_geometry_get_text(const gr_geometry *geo, uint n)
{

	return (n < geo->strings_size)? &geo->strings[n]: NULL;

}

