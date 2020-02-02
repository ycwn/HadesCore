

#ifndef GR_MESH_H
#define GR_MESH_H


enum {
	GR_MESH_VERTICES = 0x01,
	GR_MESH_INDICES  = 0x02
};

typedef struct gr_mesh {

	gr_vertexformat vformat;
	uint            istride;

	byte *p_vertices;
	byte *p_indices;

	uint n_vertices;
	uint n_indices;

	uint v_pos;
	uint i_pos;

} gr_mesh;


#define gr_mesh_new(vformat, istride, n_vertices, n_indices) gr_mesh_init(malloc(gr_mesh_size((vformat), (istride), (n_vertices), (n_indices))), (vformat), (istride), (n_vertices), (n_indices))
#define gr_mesh_tmp(vformat, istride, n_vertices, n_indices) gr_mesh_init(alloca(gr_mesh_size((vformat), (istride), (n_vertices), (n_indices))), (vformat), (istride), (n_vertices), (n_indices))


gr_mesh *gr_mesh_init(void *buf, uint vformat, uint istride, uint n_vertices, uint n_indices);
uint     gr_mesh_size(uint vformat, uint istride, uint n_vertices, uint n_indices);

void gr_mesh_rewind(gr_mesh *m, int what);

void gr_mesh_index1i(gr_mesh *m, int a);
void gr_mesh_index2i(gr_mesh *m, int a, int b);
void gr_mesh_index3i(gr_mesh *m, int a, int b, int c);
void gr_mesh_index4i(gr_mesh *m, int a, int b, int c, int d);


#define GR_MESH_VERTEX_COMPONENT(attr, count, ofs) \
	if (m->vformat.attr.components > (count)) \
		*(uint*)(m->p_vertices + (m->v_pos - 1) * m->vformat.stride + m->vformat.attr.offset + (ofs))


static inline void gr_mesh_vertex4f(gr_mesh *m, float x, float y, float z, float w) {
	m->v_pos++;
	GR_MESH_VERTEX_COMPONENT(vertex, 0, 0) = pack_r16g16_unorm(x, y);
	GR_MESH_VERTEX_COMPONENT(vertex, 2, 4) = pack_r16g16_unorm(z, w);
}

static inline void gr_mesh_texture4f(gr_mesh *m, float s, float t, float p, float q) {
	GR_MESH_VERTEX_COMPONENT(texture, 0, 0) = pack_r16g16_unorm(s, t);
	GR_MESH_VERTEX_COMPONENT(texture, 2, 4) = pack_r16g16_unorm(p, q);
}

static inline void gr_mesh_color3f(gr_mesh *m, float r, float g, float b) {
	GR_MESH_VERTEX_COMPONENT(color, 0, 0) = pack_r11g11b10f(r, g, b);
}

static inline void gr_mesh_tangent4f(gr_mesh *m, float i, float j, float k, float h) {
	GR_MESH_VERTEX_COMPONENT(tangent, 0, 0) = pack_r10g10b10a2_snorm(i, j, k, (h < 0.0f)? -1.0f: +1.0f);
}

static inline void gr_mesh_extra4f(gr_mesh *m, float a, float b, float c, float d) {
	GR_MESH_VERTEX_COMPONENT(texture, 0, 0) = pack_r16g16_sfloat(a, b);
	GR_MESH_VERTEX_COMPONENT(texture, 2, 4) = pack_r16g16_sfloat(c, d);
}

static inline void gr_mesh_bone2uf(gr_mesh *m, uint n, uint b, float w) {
	GR_MESH_VERTEX_COMPONENT(bones[n >> 1], 0, (n & 1)?4: 0) = (b & 0xffff) | (pack_r16_unorm(w) << 16);
}


static inline void gr_mesh_vertex3f(gr_mesh *m, float x, float y, float z) { gr_mesh_vertex4f(m, x, y, z,    1.0f); }
static inline void gr_mesh_vertex2f(gr_mesh *m, float x, float y)          { gr_mesh_vertex4f(m, x, y, 0.0f, 1.0f); }

static inline void gr_mesh_vertex4fv(gr_mesh *m, const float *v) { gr_mesh_vertex4f(m, v[0], v[1], v[2], v[3]); }
static inline void gr_mesh_vertex3fv(gr_mesh *m, const float *v) { gr_mesh_vertex4f(m, v[0], v[1], v[2], 1.0f); }
static inline void gr_mesh_vertex2fv(gr_mesh *m, const float *v) { gr_mesh_vertex4f(m, v[0], v[1], 0.0f, 1.0f); }

static inline void gr_mesh_vertex4v(gr_mesh *m, vec4 v) { gr_mesh_vertex4f(m, simd4f_get_x(v), simd4f_get_y(v), simd4f_get_z(v), simd4f_get_w(v)); }
static inline void gr_mesh_vertex3v(gr_mesh *m, vec4 v) { gr_mesh_vertex4f(m, simd4f_get_x(v), simd4f_get_y(v), simd4f_get_z(v), 1.0f); }
static inline void gr_mesh_vertex2v(gr_mesh *m, vec4 v) { gr_mesh_vertex4f(m, simd4f_get_x(v), simd4f_get_y(v), 0.0f,            1.0f); }


static inline void gr_mesh_texture3f(gr_mesh *m, float s, float t, float p) { gr_mesh_texture4f(m, s, t, p, 0.0f); }
static inline void gr_mesh_texture2f(gr_mesh *m, float s, float t)          { gr_mesh_texture4f(m, s, t, 0.0, 0.0f); }

static inline void gr_mesh_texture4fv(gr_mesh *m, const float *v) { gr_mesh_texture4f(m, v[0], v[1], v[2], v[3]); }
static inline void gr_mesh_texture3fv(gr_mesh *m, const float *v) { gr_mesh_texture4f(m, v[0], v[1], v[2], 0.0f); }
static inline void gr_mesh_texture2fv(gr_mesh *m, const float *v) { gr_mesh_texture4f(m, v[0], v[1], 0.0f, 0.0f); }

static inline void gr_mesh_texture4v(gr_mesh *m, vec4 v) { gr_mesh_texture4f(m, simd4f_get_x(v), simd4f_get_y(v), simd4f_get_z(v), simd4f_get_w(v)); }
static inline void gr_mesh_texture3v(gr_mesh *m, vec4 v) { gr_mesh_texture4f(m, simd4f_get_x(v), simd4f_get_y(v), simd4f_get_z(v), 0.0f); }
static inline void gr_mesh_texture2v(gr_mesh *m, vec4 v) { gr_mesh_texture4f(m, simd4f_get_x(v), simd4f_get_y(v), 0.0f,            0.0f); }


static inline void gr_mesh_color3fv(gr_mesh *m, const float *v) { gr_mesh_color3f(m, v[0],            v[1],            v[2]); }
static inline void gr_mesh_color3v( gr_mesh *m, vec4         v) { gr_mesh_color3f(m, simd4f_get_x(v), simd4f_get_y(v), simd4f_get_z(v)); }


static inline void gr_mesh_tangent4fv(gr_mesh *m, const float *v) { gr_mesh_tangent4f(m, v[0],            v[1],            v[2],            v[3]); }
static inline void gr_mesh_tangent4v( gr_mesh *m, vec4         v) { gr_mesh_tangent4f(m, simd4f_get_x(v), simd4f_get_y(v), simd4f_get_z(v), simd4f_get_w(v)); }


static inline void gr_mesh_extra3f(gr_mesh *m, float a, float b, float c) { gr_mesh_extra4f(m, a, b,    c,    0.0f); }
static inline void gr_mesh_extra2f(gr_mesh *m, float a, float b)          { gr_mesh_extra4f(m, a, b,    0.0f, 0.0f); }
static inline void gr_mesh_extra1f(gr_mesh *m, float a, float b)          { gr_mesh_extra4f(m, a, 0.0f, 0.0f, 0.0f); }

static inline void gr_mesh_extra4fv(gr_mesh *m, const float *v) { gr_mesh_extra4f(m, v[0], v[1], v[2], v[3]); }
static inline void gr_mesh_extra3fv(gr_mesh *m, const float *v) { gr_mesh_extra4f(m, v[0], v[1], v[2], 0.0f); }
static inline void gr_mesh_extra2fv(gr_mesh *m, const float *v) { gr_mesh_extra4f(m, v[0], v[1], 0.0f, 0.0f); }
static inline void gr_mesh_extra1fv(gr_mesh *m, const float *v) { gr_mesh_extra4f(m, v[0], 0.0f, 0.0f, 0.0f); }

static inline void gr_mesh_extra4v(gr_mesh *m, vec4 v) { gr_mesh_extra4f(m, simd4f_get_x(v), simd4f_get_y(v), simd4f_get_z(v), simd4f_get_w(v)); }
static inline void gr_mesh_extra3v(gr_mesh *m, vec4 v) { gr_mesh_extra4f(m, simd4f_get_x(v), simd4f_get_y(v), simd4f_get_z(v), 0.0f); }
static inline void gr_mesh_extra2v(gr_mesh *m, vec4 v) { gr_mesh_extra4f(m, simd4f_get_x(v), simd4f_get_y(v), 0.0f,            0.0f); }
static inline void gr_mesh_extra1v(gr_mesh *m, vec4 v) { gr_mesh_extra4f(m, simd4f_get_x(v), 0.0f,            0.0f,            0.0f); }


static inline void gr_mesh_point(   gr_mesh *m, int a)                      { gr_mesh_index1i(m, a);       }
static inline void gr_mesh_line(    gr_mesh *m, int a, int b)               { gr_mesh_index2i(m, a, b);    }
static inline void gr_mesh_triangle(gr_mesh *m, int a, int b, int c)        { gr_mesh_index3i(m, a, b, c); }
static inline void gr_mesh_quad3(   gr_mesh *m, int a, int b, int c, int d) { gr_mesh_index3i(m, a, b, c); gr_mesh_index3i(m, a, c, d); }
static inline void gr_mesh_quad4(   gr_mesh *m, int a, int b, int c, int d) { gr_mesh_index4i(m, a, b, c, d); }


#endif

