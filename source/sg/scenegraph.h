

#ifndef SG_SCENEGRAPH_H
#define SG_SCENEGRAPH_H


typedef struct sg_camera sg_camera;
typedef struct sg_entity sg_entity;
typedef struct sg_light  sg_light;


typedef struct sg_scenegraph {

	list cameras;
	list entities;
	list lights;

	sg_camera *camera_active;

	list node;
	char name[];

} sg_scenegraph;


sg_scenegraph *sg_scenegraph_new(const char *name);
void           sg_scenegraph_del(sg_scenegraph *sg);

sg_scenegraph *sg_scenegraph_find(const char *name);
void           sg_scenegraph_harvest();

void           sg_scenegraph_activate(sg_scenegraph *sg);
bool           sg_scenegraph_is_active(sg_scenegraph *sg);
sg_scenegraph *sg_scenegraph_get_active();


#endif


