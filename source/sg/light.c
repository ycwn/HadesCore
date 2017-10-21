

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/logger.h"

#include "sg/transform.h"
#include "sg/scenegraph.h"
#include "sg/light.h"


static list lights = LIST_INIT(&lights, NULL);



sg_light *sg_light_new(const char *name)
{

	sg_light *l = aligned_alloc(16, sizeof(sg_light) + strlen(name) + 1);

	l->color  = simd4f_zero();
	l->radius = 1.0f;

	l->transform  = NULL;
	l->scenegraph = NULL;

	list_init(&l->node,    l);
	list_init(&l->node_sg, l);

	strcpy(l->name, name);

	list_append(&lights, &l->node);

	return l;

}



void sg_light_del(sg_light *l)
{

	sg_light_detach(l);
	list_remove(&l->node);

	free(l);

}



sg_light *sg_light_find(const char *name)
{

	for (list *l=list_begin(&lights); l != list_end(&lights); l=l->next)
		if (!strcmp(LIST_PTR(sg_light, l)->name, name))
			return l->ptr;

	return NULL;

}



void sg_light_attach(sg_light *l, sg_scenegraph *sg)
{

	list_remove(&l->node_sg);

	if (sg != NULL)
		list_append(&l->node_sg, &sg->lights);

	l->scenegraph = sg;

}



void sg_light_detach(sg_light *l)
{

	list_remove(&l->node_sg);
	l->scenegraph = NULL;

}

