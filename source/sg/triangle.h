

#ifndef SG_TRIANGLE_H
#define SG_TRIANGLE_H


typedef struct sg_triangle {

	sg_entity entity;

} sg_triangle;


sg_triangle *sg_triangle_new(const char *name);
void         sg_triangle_del(sg_triangle *t);

inline void sg_triangle_attach(sg_triangle *t, sg_scenegraph *sg) { sg_entity_attach(&t->entity, sg); }
inline void sg_triangle_detach(sg_triangle *t)                    { sg_entity_detach(&t->entity); }


#endif

