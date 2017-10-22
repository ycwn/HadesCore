

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
#include "sg/entity.h"
#include "sg/triangle.h"


static const u32 verts[] = { //RG16SN R11G11B10
	0xc0000000, 0x78000000,  // XY:  0.0, -0.5; RGB: 0.0, 0.0, 1.0
	0x3fff3fff, 0x001f0000,  // XY:  0.5,  0.5; RGB: 0.0, 1.0, 0.0
	0x3fffc000, 0x000003e0   // XY: -0.5,  0.5; RGB: 1.0, 0.0, 0.0

};



sg_triangle *sg_triangle_new(const char *name)
{

	sg_triangle *t = malloc(sizeof(sg_triangle));

	t->entity = sg_entity_new(name);

	gr_vertexbuffer_init(&t->vbo);
	gr_vertexbuffer_commit_vertices(&t->vbo, verts, sizeof(verts));

	gr_command *triangle = gr_commandlist_append(&t->entity->cmds, NULL, 1, true);

	triangle->shader   = gr_shader_load("shaders/triangle.a");
	triangle->vertices = &t->vbo;
	triangle->count    = 3;

	return t;

}



void sg_triangle_del(sg_triangle *t)
{

	sg_entity_del(t->entity);
	gr_vertexbuffer_del(&t->vbo);

	free(t);

}

