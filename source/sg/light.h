

#ifndef SG_LIGHT_H
#define SG_LIGHT_H


typedef struct sg_light {

	vec4  color;
	float radius;

	sg_transform  *transform;
	sg_scenegraph *scenegraph;

	list node;
	list node_sg;

	char name[];

} sg_light;


sg_light *sg_light_new(const char *name);
void      sg_light_del(sg_light *l);

sg_light *sg_light_find(const char *name);

void sg_light_attach(sg_light *l, sg_scenegraph *sg);
void sg_light_detach(sg_light *l);


#endif

