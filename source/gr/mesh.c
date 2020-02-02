

#include "core/system.h"
#include "core/types.h"
#include "core/common.h"
#include "core/debug.h"

#include "gr/limits.h"
#include "gr/vertexformat.h"
#include "gr/mesh.h"



gr_mesh *gr_mesh_init(void *buf, uint vformat, uint istride, uint n_vertices, uint n_indices)
{

	gr_mesh *m = buf;

	gr_vf_init(&m->vformat, vformat);

	m->p_vertices = (byte*)buf + sizeof(gr_mesh);
	m->n_vertices = n_vertices;

	m->istride = istride;

	m->p_indices = m->p_vertices + m->vformat.stride * n_vertices;
	m->n_indices = n_indices;

	m->v_pos = 0;
	m->i_pos = 0;

	return m;

}



uint gr_mesh_size(uint vformat, uint istride, uint n_vertices, uint n_indices)
{

	gr_vertexformat vf;

	gr_vf_init(&vf, vformat);

	return sizeof(gr_mesh) + vf.stride * n_vertices + istride * n_indices;

}



void gr_mesh_rewind(gr_mesh *m, int what)
{

	if (what & GR_MESH_VERTICES) m->v_pos = 0;
	if (what & GR_MESH_INDICES)  m->i_pos = 0;

}



void gr_mesh_index1i(gr_mesh *m, int a)
{

	switch (m->istride) {

		case sizeof(u8):  *(u8*)( m->p_indices + m->i_pos++ * sizeof(u8))  = a; break;
		case sizeof(u16): *(u16*)(m->p_indices + m->i_pos++ * sizeof(u16)) = a; break;
		case sizeof(u32): *(u32*)(m->p_indices + m->i_pos++ * sizeof(u32)) = a; break;

	}

}



void gr_mesh_index2i(gr_mesh *m, int a, int b)
{

	switch (m->istride) {

		case sizeof(u8):
			*(u8*)(m->p_indices + m->i_pos++ * sizeof(u8)) = a;
			*(u8*)(m->p_indices + m->i_pos++ * sizeof(u8)) = b;
			break;

		case sizeof(u16):
			*(u16*)(m->p_indices + m->i_pos++ * sizeof(u16)) = a;
			*(u16*)(m->p_indices + m->i_pos++ * sizeof(u16)) = b;
			break;

		case sizeof(u32):
			*(u32*)(m->p_indices + m->i_pos++ * sizeof(u32)) = a;
			*(u32*)(m->p_indices + m->i_pos++ * sizeof(u32)) = b;
			break;

	}

}



void gr_mesh_index3i(gr_mesh *m, int a, int b, int c)
{

	switch (m->istride) {

		case sizeof(u8):
			*(u8*)(m->p_indices + m->i_pos++ * sizeof(u8)) = a;
			*(u8*)(m->p_indices + m->i_pos++ * sizeof(u8)) = b;
			*(u8*)(m->p_indices + m->i_pos++ * sizeof(u8)) = c;
			break;

		case sizeof(u16):
			*(u16*)(m->p_indices + m->i_pos++ * sizeof(u16)) = a;
			*(u16*)(m->p_indices + m->i_pos++ * sizeof(u16)) = b;
			*(u16*)(m->p_indices + m->i_pos++ * sizeof(u16)) = c;
			break;

		case sizeof(u32):
			*(u32*)(m->p_indices + m->i_pos++ * sizeof(u32)) = a;
			*(u32*)(m->p_indices + m->i_pos++ * sizeof(u32)) = b;
			*(u32*)(m->p_indices + m->i_pos++ * sizeof(u32)) = c;
			break;

	}

}



void gr_mesh_index4i(gr_mesh *m, int a, int b, int c, int d)
{

	switch (m->istride) {

		case sizeof(u8):
			*(u8*)(m->p_indices + m->i_pos++ * sizeof(u8)) = a;
			*(u8*)(m->p_indices + m->i_pos++ * sizeof(u8)) = b;
			*(u8*)(m->p_indices + m->i_pos++ * sizeof(u8)) = c;
			*(u8*)(m->p_indices + m->i_pos++ * sizeof(u8)) = d;
			break;

		case sizeof(u16):
			*(u16*)(m->p_indices + m->i_pos++ * sizeof(u16)) = a;
			*(u16*)(m->p_indices + m->i_pos++ * sizeof(u16)) = b;
			*(u16*)(m->p_indices + m->i_pos++ * sizeof(u16)) = c;
			*(u16*)(m->p_indices + m->i_pos++ * sizeof(u16)) = d;
			break;

		case sizeof(u32):
			*(u32*)(m->p_indices + m->i_pos++ * sizeof(u32)) = a;
			*(u32*)(m->p_indices + m->i_pos++ * sizeof(u32)) = b;
			*(u32*)(m->p_indices + m->i_pos++ * sizeof(u32)) = c;
			*(u32*)(m->p_indices + m->i_pos++ * sizeof(u32)) = d;
			break;

	}

}

