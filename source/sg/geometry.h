

#ifndef SG_GEOMETRY_H
#define SG_GEOMETRY_H


typedef struct sg_geometry {

	sg_entity   entity;
	gr_geometry geometry;

} sg_geometry;


sg_geometry *sg_geometry_new(const char *name);
void         sg_geometry_del(sg_geometry *g);
bool         sg_geometry_load(  sg_geometry *g, const char *file);
bool         sg_geometry_loadfd(sg_geometry *g, int id);

static inline void sg_geometry_attach(sg_geometry *g, sg_scenegraph *sg) { sg_entity_attach(&g->entity, sg); }
static inline void sg_geometry_detach(sg_geometry *g)                    { sg_entity_detach(&g->entity); }


#endif

