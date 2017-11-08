

#ifndef SG_CUBE_H
#define SG_CUBE_H


typedef struct sg_cube {

	sg_entity entity;

} sg_cube;


sg_cube *sg_cube_new(const char *name);
void     sg_cube_del(sg_cube *c);

inline void sg_cube_attach(sg_cube *c, sg_scenegraph *sg) { sg_entity_attach(&c->entity, sg); }
inline void sg_cube_detach(sg_cube *c)                    { sg_entity_detach(&c->entity); }


#endif

