

#ifndef SG_CAMERA_H
#define SG_CAMERA_H


typedef struct sg_camera {

	mat4 projection;

	sg_transform  *transform;
	sg_scenegraph *scenegraph;

	list node;
	list node_sg;

	char name[];

} sg_camera;


sg_camera *sg_camera_new(const char *name);
void       sg_camera_del(sg_camera *c);

sg_camera *sg_camera_find(const char *name);

void sg_camera_activate(sg_camera *c);
bool sg_camera_is_active(sg_camera *c);

void sg_camera_attach(sg_camera *c, sg_scenegraph *sg);
void sg_camera_detach(sg_camera *c);

void sg_camera_ortho(sg_camera *c, float width, float height, float near, float far);
void sg_camera_perspective(sg_camera *c, float fov, float near, float far);


#endif

