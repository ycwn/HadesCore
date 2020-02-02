

#include "core/system.h"
#include "core/types.h"
#include "core/common.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/logger.h"

#include "sg/transform.h"


typedef struct sg_transform_node {

	sg_transform transform;

	mat4  rotation;
	vec4  translation;
	float scale;

	struct sg_transform_node *parent;

	list kids;
	list node;
	list node_xforms;
	list node_queue;

	bool modified;
	char name[];

} sg_transform_node;


static list transforms = LIST_INIT(&transforms, NULL);
static list queue      = LIST_INIT(&queue,      NULL);

static void update_transform_hierarchy(sg_transform_node *xf);



sg_transform *sg_transform_new(const char *name)
{

	sg_transform_node *xf = aligned_alloc(16, sizeof(sg_transform_node) + strlen(name) + 1);

	simd4x4f_identity(&xf->transform.local);
	simd4x4f_identity(&xf->transform.model);
	simd4x4f_identity(&xf->rotation);

	xf->translation = simd4f_zero();
	xf->scale       = 1.0f;

	xf->parent = NULL;

	list_init(&xf->kids,        NULL);
	list_init(&xf->node,        xf);
	list_init(&xf->node_xforms, xf);
	list_init(&xf->node_queue,  xf);

	xf->modified = false;

	strcpy(xf->name, name);

	list_append(&transforms, &xf->node_xforms);

	return &xf->transform;

}



void sg_transform_del(sg_transform *_xf)
{

	sg_transform_node *xf = (sg_transform_node*)_xf;

	list_remove(&xf->node);
	list_remove(&xf->node_queue);
	list_remove(&xf->node_xforms);

	while (!list_empty(&xf->kids))
		sg_transform_del(list_front(&xf->kids));

	free(xf);

}



sg_transform *sg_transform_find(const char *name)
{

	for (list *xf=list_begin(&transforms); xf != list_end(&transforms); xf=xf->next)
		if (!strcmp(LIST_PTR(sg_transform_node, xf)->name, name))
			return xf->ptr;

	return NULL;

}



void sg_transform_update()
{

	while (!list_empty(&queue))
		update_transform_hierarchy(list_front(&queue));

}



void sg_transform_attach(sg_transform *_xf, sg_transform *parent)
{

	sg_transform_node *xf = (sg_transform_node*)_xf;

	list_remove(&xf->node);
	list_append(&xf->node_queue, &queue);

	xf->parent = (sg_transform_node*)parent;

	if (xf->parent != NULL)
		list_append(&xf->node, &xf->parent->kids);

}



void sg_transform_detach(sg_transform *_xf)
{

	sg_transform_node *xf = (sg_transform_node*)_xf;

	list_remove(&xf->node);
	list_append(&xf->node_queue, &queue);

	xf->parent = NULL;

}



sg_transform *sg_transform_find_child(sg_transform *_xf, const char *name)
{

	sg_transform_node *xf = (sg_transform_node*)_xf;

	for (list *kid=list_begin(&xf->kids); kid != list_end(&xf->kids); kid=kid->next)
		if (!strcmp(LIST_PTR(sg_transform_node, kid)->name, name))
			return kid->ptr;

	return NULL;

}



void sg_transform_lookat(sg_transform *_xf, const vec4 origin, const vec4 target, const vec4 up)
{

	sg_transform_node *xf = (sg_transform_node*)_xf;

	simd4x4f_lookat(&xf->rotation, simd4f_zero(), simd4f_sub(target, origin), up);

	xf->translation = origin;
	xf->modified    = true;

	if (!list_connected(&xf->node_queue))
		list_append(&xf->node_queue, &queue);

}



void sg_transform_rotate_axis(sg_transform *_xf, const vec4 axis, const float theta)
{

	const float cos_th = cosf(theta / 2.0f);
	const float sin_th = sinf(theta / 2.0f);

	sg_transform_rotate_quaternion(_xf,
		simd4f_madd(axis,
			    simd4f_create(sin_th, sin_th, sin_th, 0.0f),
			    simd4f_create(0.0f,   0.0f,   0.0f,   cos_th)));

}



void sg_transform_rotate_angle(sg_transform *_xf, const float phi, const float theta)
{

	const float cos_th = cosf(theta / 2.0f), cos_ph = cosf(phi / 2.0f);
	const float sin_th = sinf(theta / 2.0f), sin_ph = sinf(phi / 2.0f);

	sg_transform_rotate_quaternion(_xf,
		simd4f_create(
			sin_th * cos_ph,
			cos_th * sin_ph,
			sin_th * sin_ph,
			cos_th * cos_ph));

}



void sg_transform_rotate_quaternion(sg_transform *_xf, const vec4 quaternion)
{

	sg_transform_node *xf = (sg_transform_node*)_xf;

	const float qx = simd4f_get_x(quaternion); //FIXME: Optimize
	const float qy = simd4f_get_y(quaternion); //FIXME: Might need transpose
	const float qz = simd4f_get_z(quaternion);
	const float qw = simd4f_get_w(quaternion);

	xf->rotation.x = simd4f_create(
		1.0f - 2.0f * (qy * qy + qz * qz),
		       2.0f * (qx * qy + qz * qw),
		       2.0f * (qx * qz - qy * qw),
		0.0f);

	xf->rotation.y = simd4f_create(
		       2.0f * (qx * qy - qz * qw),
		1.0f - 2.0f * (qx * qx + qz * qz),
		       2.0f * (qy * qz + qx * qw),
		0.0f);

	xf->rotation.z = simd4f_create(
		       2.0f * (qx * qz + qy * qw),
		       2.0f * (qy * qz - qx * qw),
		1.0f - 2.0f * (qx * qx + qy * qy),
		0.0f);

	xf->rotation.w = simd4f_create(0.0f, 0.0f, 0.0f, 1.0f);
	xf->modified   = true;

	if (!list_connected(&xf->node_queue))
		list_append(&queue, &xf->node_queue);

}



void sg_transform_rotate_matrix(sg_transform *_xf, const mat4 *matrix)
{

	sg_transform_node *xf = (sg_transform_node*)_xf;

	xf->rotation = *matrix;
	xf->modified = true;

	if (!list_connected(&xf->node_queue))
		list_append(&queue, &xf->node_queue);

}



void sg_transform_translate(sg_transform *_xf, const vec4 translation)
{

	sg_transform_node *xf = (sg_transform_node*)_xf;

	xf->translation = simd4f_zero_w(translation);
	xf->modified    = true;

	if (!list_connected(&xf->node_queue))
		list_append(&queue, &xf->node_queue);

}



void sg_transform_scale(sg_transform *_xf, const float scale)
{

	sg_transform_node *xf = (sg_transform_node*)_xf;

	xf->scale    = scale;
	xf->modified = true;

	if (!list_connected(&xf->node_queue))
		list_append(&queue, &xf->node_queue);

}



void sg_transform_modify(sg_transform *_xf)
{

	sg_transform_node *xf = (sg_transform_node*)_xf;

	xf->modified = true;

	if (!list_connected(&xf->node_queue))
		list_append(&queue, &xf->node_queue);

}



const char *sg_transform_get_name(sg_transform *xf)
{

	return ((sg_transform_node*)xf)->name;

}



sg_transform *sg_transform_get_parent( sg_transform *xf)
{

	return &((sg_transform_node*)xf)->parent->transform;

}



bool sg_transform_has_parent(sg_transform *xf)
{

	return ((sg_transform_node*)xf)->parent != NULL;

}



bool sg_transform_has_kids(sg_transform *xf)
{

	return !list_empty(&((sg_transform_node*)xf)->kids);

}



bool sg_transform_is_modified(sg_transform *xf)
{

	return ((sg_transform_node*)xf)->modified;

}



void update_transform_hierarchy(sg_transform_node *xf)
{

	if (xf->modified) {

		const vec4 scale = simd4f_zero_w(simd4f_splat(xf->scale));
		const vec4 mask  = simd4f_create(0.0f, 0.0f, 0.0f, 1.0f);

		xf->transform.local.x = simd4f_mul(scale, xf->rotation.x);
		xf->transform.local.y = simd4f_mul(scale, xf->rotation.y);
		xf->transform.local.z = simd4f_mul(scale, xf->rotation.z);
		xf->transform.local.w = simd4f_add(xf->translation, mask);

	}

	if (xf->parent != NULL)
		simd4x4f_matrix_mul(&xf->transform.local, &xf->parent->transform.model, &xf->transform.model);

	else
		xf->transform.model = xf->transform.local;


	list_remove(&xf->node_queue);
	xf->modified = false;

	for (list *kid=list_begin(&xf->kids); kid != list_end(&xf->kids); kid=kid->next)
		update_transform_hierarchy(kid->ptr);

}

