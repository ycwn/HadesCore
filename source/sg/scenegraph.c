

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/string.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/rendertarget.h"
#include "gr/vertexformat.h"
#include "gr/shader.h"
#include "gr/vertexbuffer.h"
#include "gr/uniformbuffer.h"
#include "gr/command.h"
#include "gr/commandqueue.h"
#include "gr/commandlist.h"

#include "sg/transform.h"
#include "sg/scenegraph.h"
#include "sg/camera.h"
#include "sg/entity.h"
#include "sg/light.h"


static list           scenegraphs = LIST_INIT(&scenegraphs, NULL);
static sg_scenegraph *active = NULL;



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
}



void sg_scenegraph_activate(sg_scenegraph *sg)
{

	active = sg;

}



bool sg_scenegraph_is_active(sg_scenegraph *sg)
{

	return active == sg;

}



sg_scenegraph* sg_scenegraph_get_active()
{

	return active;

}

