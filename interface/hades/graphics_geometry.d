

import hades.core_types;

import hades.graphics_commandlist;
import hades.graphics_vertexbuffer;
import hades.graphics_vertexformat;

import gpuio;


struct gr_geometry {

	gr_vertexformat vf;
	uint            index_size;

	const(void) *vertices;
	uint         vertices_count;
	uint         vertices_size;

	const(void) *indices;
	uint         indices_count;
	uint         indices_size;

	const(void) *drawcalls;
	uint         drawcalls_count;

	const(char) *strings;
	uint         strings_size;

}


struct gr_material {

	const(char) *name;
	const(char) *shader;

	vec4 minimum;
	vec4 maximum;

	vec4 albedo;
	vec4 emission;

	float transparency;
	float metallicity;
	float roughness;
	float refraction;

	vec4[5] args;

}


extern(C) {

	void gr_geometry_init(  gr_geometry *geo);
	bool gr_geometry_open(  gr_geometry *geo, const(char) *file);
	bool gr_geometry_openfd(gr_geometry *geo, int id);
	bool gr_geometry_load(  gr_geometry *geo, const(void) *ptr);
	void gr_geometry_commit_drawcalls(const(gr_geometry) *geo, gr_commandlist  *cmdl);
	void gr_geometry_commit_buffers(  const(gr_geometry) *geo, gr_vertexbuffer *vb);
	void gr_geometry_commit_uniforms( const(gr_geometry) *geo, gpu_uniform_material *ub, uint n);

	const(gr_material) *gr_geometry_get_material(const(gr_geometry) *geo, gr_material *tmp, uint n);
	const(char)        *gr_geometry_get_text(const(gr_geometry) *geo, uint n);

}

