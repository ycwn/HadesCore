

#ifndef SG_ENTITY_H
#define SG_ENTITY_H


typedef struct sg_entity {

	gr_commandlist cmds;

	sg_transform  *transform;
	sg_scenegraph *scenegraph;

	list node;
	list node_sg;

	char name[];

} sg_entity;


sg_entity *sg_entity_new(const char *name);
void       sg_entity_del(sg_entity *e);

sg_entity *sg_entity_find(const char *name);

void sg_entity_attach(sg_entity *e, sg_scenegraph *sg);
void sg_entity_detach(sg_entity *e);



#endif

