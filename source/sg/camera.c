

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/logger.h"

#include "sg/transform.h"
#include "sg/scenegraph.h"
#include "sg/camera.h"


static list cameras = LIST_INIT(&cameras, NULL);



sg_camera *sg_camera_new(const char *name)
{

	sg_camera *c = aligned_alloc(16, sizeof(sg_camera) + strlen(name) + 1);

	simd4x4f_identity(&c->projection);

	c->transform  = NULL;
	c->scenegraph = NULL;

	list_init(&c->node,    c);
	list_init(&c->node_sg, c);

	strcpy(c->name, name);

	list_append(&cameras, &c->node);

	return c;

}



void sg_camera_del(sg_camera *c)
{

	sg_camera_detach(c);
	list_remove(&c->node);

	free(c);

}



sg_camera *sg_camera_find(const char *name)
{

	for (list *c=list_begin(&cameras); c != list_end(&cameras); c=c->next)
		if (!strcmp(LIST_PTR(sg_camera, c)->name, name))
			return c->ptr;

	return NULL;

}



void sg_camera_activate(sg_camera *c)
{

	if (c->scenegraph != NULL)
		c->scenegraph->camera_active = c;

}



bool sg_camera_is_active(sg_camera *c)
{

	return c->scenegraph != NULL && c->scenegraph->camera_active == c;

}



void sg_camera_attach(sg_camera *c, sg_scenegraph *sg)
{

	list_remove(&c->node_sg);

	if (sg != NULL)
		list_append(&c->node_sg, &sg->cameras);

	c->scenegraph = sg;

}



void sg_camera_detach(sg_camera *c)
{

	if (c->scenegraph != NULL && c->scenegraph->camera_active == c)
		c->scenegraph->camera_active = NULL;

	list_remove(&c->node_sg);

	c->scenegraph = NULL;

}



void sg_camera_ortho(sg_camera *c, float width, float height, float near, float far)
{

	const float field_w = 2.0f / width;
	const float field_h = 2.0f / height;
	const float depth_a = 2.0f / (near - far);
	const float depth_b = (near + far) / (near - far);

	c->projection = simd4x4f_create(
		simd4f_create(field_w,     0.0f,    0.0f, 0.0f),
		simd4f_create(0.0f,    -field_h,    0.0f, 0.0f),
		simd4f_create(0.0f,        0.0f, depth_a, 0.0f),
		simd4f_create(0.0f,        0.0f, depth_b, 1.0f));

}



void sg_camera_perspective(sg_camera *c, float fov, float near, float far)
{

	const float field   = 1.0f / tanf(fov * M_PI / 360.0f);
	const float depth_a = (near + far) / (near - far);
	const float depth_b = 2.0f * near * far / (near - far);

	c->projection = simd4x4f_create(
		simd4f_create(field,  0.0f,    0.0f,  0.0f),
		simd4f_create(0.0f,  -field,   0.0f,  0.0f),
		simd4f_create(0.0f,   0.0f, depth_a, -1.0f),
		simd4f_create(0.0f,   0.0f, depth_b,  1.0f));

}

