

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
#include "gr/vertexbuffer.h"
#include "gr/uniformbuffer.h"
#include "gr/shader.h"
#include "gr/command.h"
#include "gr/commandqueue.h"
#include "gr/commandlist.h"

#include "sg/transform.h"
#include "sg/scenegraph.h"
#include "sg/entity.h"


static list entities = LIST_INIT(&entities, NULL);



sg_entity *sg_entity_new(const char *name, size_t extra)
{

	sg_entity *e = aligned_alloc(16, sizeof(sg_entity) + extra + strlen(name) + 1);

	gr_commandlist_init(&e->cmds);
	gr_vertexbuffer_init(&e->vbo);
	gr_uniformbuffer_init(&e->ubo);

	e->cmds.vertices = &e->vbo;
	e->cmds.uniforms = &e->ubo;

	e->transform  = NULL;
	e->scenegraph = NULL;
	e->name       = (char*)&e->extra[0] + extra;

	list_init(&e->node,    e);
	list_init(&e->node_sg, e);

	e->recalculate = false;

	strcpy(e->name, name);

	list_append(&entities, &e->node);

	return e;

}



void sg_entity_del(sg_entity *e)
{

	gr_commandlist_del(&e->cmds);
	gr_vertexbuffer_del(&e->vbo);
	gr_uniformbuffer_del(&e->ubo);

	sg_entity_detach(e);

	list_remove(&e->node);
	free(e);

}



sg_entity *sg_entity_find(const char *name)
{

	for (list *e=list_begin(&entities); e != list_end(&entities); e=e->next)
		if (!strcmp(LIST_PTR(sg_entity, e)->name, name))
			return e->ptr;

	return NULL;

}



void sg_entity_attach(sg_entity *e, sg_scenegraph *sg)
{

	list_remove(&e->node_sg);

	if (sg != NULL)
		list_append(&e->node_sg, &sg->entities);

	e->scenegraph = sg;

}



void sg_entity_detach(sg_entity *e)
{

	list_remove(&e->node_sg);
	e->scenegraph = NULL;

}

