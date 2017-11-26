

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/rendertarget.h"
#include "gr/vertexformat.h"
#include "gr/vertexbuffer.h"
#include "gr/uniformbuffer.h"
#include "gr/shader.h"
#include "gr/command.h"
#include "gr/commandqueue.h"
#include "gr/commandlist.h"

#include "sg/transform.h"
#include "sg/scenegraph.h"
#include "sg/camera.h"
#include "sg/entity.h"
#include "sg/light.h"


static list           scenegraphs = LIST_INIT(&scenegraphs, NULL);
static sg_scenegraph *active      = NULL;



sg_scenegraph *sg_scenegraph_new(const char *name)
{

	sg_scenegraph *sg = aligned_alloc(16, sizeof(sg_scenegraph) + strlen(name) + 1);

	list_init(&sg->cameras,  NULL);
	list_init(&sg->entities, NULL);
	list_init(&sg->lights,   NULL);

	sg->camera_active = NULL;

	list_init(&sg->node, sg);
	list_append(&scenegraphs, &sg->node);

	strcpy(sg->name, name);
	return sg;

}



void sg_scenegraph_del(sg_scenegraph *sg)
{

	list_remove(&sg->node);

	if (active == sg)
		active = NULL;

	while (!list_empty(&sg->cameras))
		sg_camera_detach(list_front(&sg->cameras));

	while (!list_empty(&sg->entities))
		sg_entity_detach(list_front(&sg->entities));

	while (!list_empty(&sg->lights))
		sg_light_detach(list_front(&sg->lights));

	free(sg);

}



sg_scenegraph *sg_scenegraph_find(const char *name)
{

	for (list *sg=list_begin(&scenegraphs); sg != list_end(&scenegraphs); sg=sg->next)
		if (!strcmp(LIST_PTR(sg_scenegraph, sg)->name, name))
			return sg->ptr;

	return NULL;

}



void sg_scenegraph_harvest()
{

	mat4 eye;
	mat4 view;
	mat4 proj;
	mat4 view_proj;
	mat4 identity;
	bool eye_modified = false;

	if (active == NULL)
		return;

	simd4x4f_identity(&identity);

	if (active->camera_active != NULL) {

		if (active->camera_active->transform != NULL) {

			sg_transform *xf = active->camera_active->transform;

			eye = xf->model;

			view = eye;
			view.w = simd4f_splat(0.0f);

			simd4x4f_transpose_inplace(&view);
			simd4x4f_matrix_vector3_mul(&view, &eye.w, &view.w);
			view.w = simd4f_sub(simd4f_create(0.0f, 0.0f, 0.0f, 1.0f), view.w);

			eye_modified = sg_transform_is_modified(xf);

		} else {

			simd4x4f_identity(&eye);
			simd4x4f_identity(&view);

		}

		proj = active->camera_active->projection;

	} else {

		simd4x4f_identity(&eye);
		simd4x4f_identity(&view);
		simd4x4f_identity(&proj);

	}

	simd4x4f_matrix_mul(&view, &proj, &view_proj);


	for (list *e=list_begin(&active->entities); e != list_end(&active->entities); e=e->next) {

		sg_entity *ent = LIST_PTR(sg_entity,e);

		mat4 *model    = &identity;
		bool  modified = ent->recalculate || eye_modified;

		if (ent->transform != NULL) {

			model     = &ent->transform->model;
			modified |= sg_transform_is_modified(ent->transform);

		}

		if (modified) {

			gpu_uniform_object u;

			szero(u);

			u.mat_model  = *model;
			u.mat_view   = view;
			u.mat_proj   = proj;
			u.mat_eye    = eye;
			u.mat_viewproj = view_proj;

			simd4x4f_matrix_mul(model, &view,      &u.mat_modelview);
			simd4x4f_matrix_mul(model, &view_proj, &u.mat_mvp);

			gr_uniformbuffer_commit(&ent->ubo, &u, sizeof(u));

			ent->recalculate = false;

		}

		gr_commandlist_enqueue(&ent->cmds);

	}

}



void sg_scenegraph_activate(sg_scenegraph *sg)
{

	active = sg;

}



bool sg_scenegraph_is_active(sg_scenegraph *sg)
{

	return active == sg;

}



sg_scenegraph *sg_scenegraph_get_active()
{

	return active;

}

