

#ifndef SG_SKYLIGHT_H
#define SG_SKYLIGHT_H


typedef struct sg_skylight {

	sg_entity  entity;

	float turbidity;
	float albedo;
	float elevation;

} sg_skylight;


sg_skylight *sg_skylight_new(const char *name);
void         sg_skylight_del(sg_skylight *s);
bool         sg_skylight_load(  sg_skylight *s, const char *model);
bool         sg_skylight_loadfd(sg_skylight *s, int id);

void sg_skylight_configure(sg_skylight *s);

int  sg_skylight_get_tesselation_level(const sg_skylight *s);
void sg_skylight_set_tesselation_level(const sg_skylight *s, int level);

void sg_skylight_sample(const sg_skylight *s, V3 color, float theta, float gamma);
void sg_skylight_sun(   const sg_skylight *s, V3 color, float theta);

static inline void sg_skylight_attach(sg_skylight *s, sg_scenegraph *sg) { sg_entity_attach(&s->entity, sg); }
static inline void sg_skylight_detach(sg_skylight *s)                    { sg_entity_detach(&s->entity); }


#endif

