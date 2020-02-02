

#include "core/system.h"
#include "core/types.h"
#include "core/common.h"
#include "core/debug.h"
#include "core/blob.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/pixelformat.h"
#include "gr/vertexformat.h"
#include "gr/mesh.h"
#include "gr/surface.h"
#include "gr/renderpass.h"
#include "gr/vertexbuffer.h"
#include "gr/uniformbuffer.h"
#include "gr/shader.h"
#include "gr/command.h"
#include "gr/commandqueue.h"
#include "gr/commandlist.h"

#include "sg/transform.h"
#include "sg/scenegraph.h"
#include "sg/entity.h"
#include "sg/cubesphere.h"


struct cubeface {

	uint du_p, du_m;
	uint dv_p, dv_m;

	V3 normal;

};

static const struct cubeface cubemap_faces[6]={
	{ 4, 5, 3, 2, { +1.0f,  0.0f,  0.0f } },
	{ 5, 4, 3, 2, { -1.0f,  0.0f,  0.0f } },
	{ 4, 5, 0, 1, {  0.0f, +1.0f,  0.0f } },
	{ 4, 5, 1, 0, {  0.0f, -1.0f,  0.0f } },
	{ 1, 0, 3, 2, {  0.0f,  0.0f, +1.0f } },
	{ 0, 1, 3, 2, {  0.0f,  0.0f, -1.0f } }
};



sg_cubesphere *sg_cubesphere_new(const char *name)
{

	sg_cubesphere *c = (sg_cubesphere*)sg_entity_new(name, sizeof(sg_cubesphere) - sizeof(sg_entity));

	c->mesh = NULL;

	c->lod_min = 0;
	c->lod_max = 0;

	for (int n=0; n < SG_CUBESPHERE_LOD_LEVELS; n++) {

		c->lod_start[n] = 0;
		c->lod_count[n] = 0;
		c->lod_error[n] = 0.0f;

	}

	return c;

}



void sg_cubesphere_del(sg_cubesphere *c)
{

	free(c->mesh);

	sg_entity_del(&c->entity);

}



void sg_cubesphere_tesselate(sg_cubesphere *c, int lod_min, int lod_max)
{

	free(c->mesh);

	uint n_vertex = 0;
	uint n_index  = 0;


	for (uint lod=lod_min; lod <= lod_max; lod++) {

		const uint n_edge_verts = 1 << (lod + 1);
		const uint n_edge_faces = n_edge_verts - 1;

		n_vertex += 6 * n_edge_verts * n_edge_verts;
		n_index  += 6 * n_edge_faces * n_edge_faces * 6;

	}

	c->mesh = gr_mesh_new(GR_V3 | GR_T2 | GR_Q4, (n_vertex < 65536)? sizeof(u16): sizeof(u32), n_vertex, n_index);

	const vec4 ds_p = simd4f_uload3(cubemap_faces[cubemap_faces[4].du_p].normal);
	const vec4 ds_m = simd4f_uload3(cubemap_faces[cubemap_faces[4].du_m].normal);
	const vec4 dt_p = simd4f_uload3(cubemap_faces[cubemap_faces[4].dv_p].normal);
	const vec4 dt_m = simd4f_uload3(cubemap_faces[cubemap_faces[4].dv_m].normal);

	const vec4 up = simd4f_create(0.0f, 1.0f, 0.0f, 0.0f);


	for (uint lod=lod_min; lod <= lod_max; lod++) {

		const uint n_edge_verts = 1 << (lod + 1);
		const uint n_edge_faces = n_edge_verts - 1;
		const uint n_side_verts = n_edge_verts * n_edge_verts;

		const uint v_base = c->mesh->v_pos;
		const uint i_base = c->mesh->i_pos;

		for (uint s=0; s < 6; s++) {

			const vec4 du_p = simd4f_uload3(cubemap_faces[cubemap_faces[s].du_p].normal);
			const vec4 du_m = simd4f_uload3(cubemap_faces[cubemap_faces[s].du_m].normal);
			const vec4 dv_p = simd4f_uload3(cubemap_faces[cubemap_faces[s].dv_p].normal);
			const vec4 dv_m = simd4f_uload3(cubemap_faces[cubemap_faces[s].dv_m].normal);
			const vec4 norm = simd4f_uload3(cubemap_faces[s].normal);

			for (uint v=0; v < n_edge_verts; v++)
				for (uint u=0; u < n_edge_verts; u++) {

					const float du = (float)u / (float)(n_edge_verts - 1);
					const float dv = (float)v / (float)(n_edge_verts - 1);

					const vec4 du0 = simd4f_splat(1.0f - du), du1 = simd4f_splat(du);
					const vec4 dv0 = simd4f_splat(1.0f - dv), dv1 = simd4f_splat(dv);

					const vec4 V =
						simd4f_madd(
							simd4f_splat(0.5f),
							simd4f_normalize3(
								simd4f_add(
									norm,
									simd4f_add(
										simd4f_madd(du_m, du0, simd4f_mul(du_p, du1)),
										simd4f_madd(dv_m, dv0, simd4f_mul(dv_p, dv1))))),
							simd4f_splat(0.5f));

					const vec4 T =
						simd4f_madd(
							simd4f_splat(0.5f),
							simd4f_add(
								simd4f_madd(ds_m, du0, simd4f_mul(ds_p, du1)),
								simd4f_madd(dt_m, dv0, simd4f_mul(dt_p, dv1))),
							simd4f_splat(0.5f));

					gr_mesh_vertex3v( c->mesh, V);
					gr_mesh_texture2v(c->mesh, T);
					gr_mesh_tangent4v(c->mesh, calculate_tangent_q4h(V, up));

				}

			for (uint v=0; v < n_edge_faces; v++)
				for (uint u=0; u < n_edge_faces; u++)
					gr_mesh_quad3(c->mesh,
						v_base + n_side_verts * s + (v + 1) * n_edge_verts + (u + 0),
						v_base + n_side_verts * s + (v + 1) * n_edge_verts + (u + 1),
						v_base + n_side_verts * s + (v + 0) * n_edge_verts + (u + 1),
						v_base + n_side_verts * s + (v + 0) * n_edge_verts + (u + 0));

		}

		c->lod_start[lod] = i_base;
		c->lod_count[lod] = c->mesh->i_pos - i_base;
		c->lod_error[lod] = 1.0f - cosf(M_PI_4 / (float)n_edge_faces);

	}

	gr_vertexbuffer_commit_vertices(&c->entity.vbo, c->mesh->p_vertices, c->mesh->n_vertices * c->mesh->vformat.stride);
	gr_vertexbuffer_commit_indices( &c->entity.vbo, c->mesh->p_indices,  c->mesh->n_indices  * c->mesh->istride);

	c->lod_min = lod_min;
	c->lod_max = lod_max;

	sg_cubesphere_set_lod(c, lod_min);

}



void sg_cubesphere_set_lod(sg_cubesphere *c, int lod)
{

	c->lod = clampi(lod, c->lod_min, c->lod_max);

	for (gr_command *cmd=gr_commandlist_begin(&c->entity.cmds); cmd != gr_commandlist_end(&c->entity.cmds); cmd++) {

		cmd->start = c->lod_start[c->lod];
		cmd->count = c->lod_count[c->lod];

	}

}



gr_command *sg_cubesphere_add_shader(sg_cubesphere *c, gr_shader *s)
{

	gr_command *cmd = gr_commandlist_append(&c->entity.cmds, s, 1, true);

	if (cmd != NULL) {

		cmd->start = c->lod_start[c->lod];
		cmd->count = c->lod_count[c->lod];

	}

}

