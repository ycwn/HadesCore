

import hades.graphics_command;
import hades.graphics_shader;

import hades.scenegraph_entity;
import hades.scenegraph_scenegraph;


enum SG_CUBESPHERE_LOD_LEVELS = 20;


struct sg_cubesphere {

	sg_entity  entity;
	void      *mesh;

	uint lod_min;
	uint lod_max;
	uint lod;

	uint[SG_CUBESPHERE_LOD_LEVELS]  lod_start;
	uint[SG_CUBESPHERE_LOD_LEVELS]  lod_count;
	float[SG_CUBESPHERE_LOD_LEVELS] lod_error;

}


extern(C) {

	sg_cubesphere *sg_cubesphere_new(const char *name);
	void           sg_cubesphere_del(sg_cubesphere *c);

	void sg_cubesphere_tesselate(sg_cubesphere *c, int lod_min, int lod_max);
	void sg_cubesphere_set_lod(  sg_cubesphere *c, int lod);

	gr_command *sg_cubesphere_add_shader(sg_cubesphere *c, gr_shader *s);

}

void sg_cubesphere_attach(sg_cubesphere *c, sg_scenegraph *sg) { sg_entity_attach(&c.entity, sg); }
void sg_cubesphere_detach(sg_cubesphere *c)                    { sg_entity_detach(&c.entity); }

