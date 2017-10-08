

#ifndef SG_TRANSFORM_H
#define SG_TRANSFORM_H


typedef struct sg_transform {

	mat4 local;
	mat4 model;

} sg_transform;


sg_transform *sg_transform_new(const char *name);
void          sg_transform_del(sg_transform *xf);

sg_transform *sg_transform_find(const char *name);
void          sg_transform_update();

void sg_transform_attach(sg_transform *xf, sg_transform *parent);
void sg_transform_detach(sg_transform *xf);

sg_transform *sg_transform_find_child(sg_transform *xf, const char *name);

void sg_transform_lookat(sg_transform *xf, const vec4 origin, const vec4 target, const vec4 up);
void sg_transform_rotate_axis( sg_transform *xf, const vec4 axis, const float theta);
void sg_transform_rotate_angle(sg_transform *xf, const float phi, const float theta);
void sg_transform_rotate_quaternion(sg_transform *xf, const vec4 quaternion);
void sg_transform_rotate_matrix(    sg_transform *xf, const mat4 *matrix);
void sg_transform_translate(sg_transform *xf, const vec4 translation);
void sg_transform_scale(    sg_transform *xf, const float scale);
void sg_transform_modify(   sg_transform *xf);

const char   *sg_transform_get_name(   sg_transform *xf);
sg_transform *sg_transform_get_parent( sg_transform *xf);
bool          sg_transform_has_parent( sg_transform *xf);
bool          sg_transform_has_kids(   sg_transform *xf);
bool          sg_transform_is_modified(sg_transform *xf);


#endif


