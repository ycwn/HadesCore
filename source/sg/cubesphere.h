

#ifndef SG_CUBESPHERE_H
#define SG_CUBESPHERE_H


enum {
	SG_CUBESPHERE_LOD_LEVELS = 20
};


typedef struct sg_cubesphere {

	sg_entity  entity;
	gr_mesh   *mesh;

	uint lod_min;
	uint lod_max;
	uint lod;

	uint  lod_start[SG_CUBESPHERE_LOD_LEVELS];
	uint  lod_count[SG_CUBESPHERE_LOD_LEVELS];
	float lod_error[SG_CUBESPHERE_LOD_LEVELS];

} sg_cubesphere;


sg_cubesphere *sg_cubesphere_new(const char *name);
void           sg_cubesphere_del(sg_cubesphere *c);

void sg_cubesphere_tesselate(sg_cubesphere *c, int lod_min, int lod_max);
void sg_cubesphere_set_lod(  sg_cubesphere *c, int lod);

gr_command *sg_cubesphere_add_shader(sg_cubesphere *c, gr_shader *s);


static inline void sg_cubesphere_attach(sg_cubesphere *c, sg_scenegraph *sg) { sg_entity_attach(&c->entity, sg); }
static inline void sg_cubesphere_detach(sg_cubesphere *c)                    { sg_entity_detach(&c->entity); }


#endif

