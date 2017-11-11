

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
#include "sg/entity.h"
#include "sg/cube.h"


static const u32 verts[] = { //RGBA16SN R11G11B10
	0xc000c000, 0x7fffc000, 0x00000000,  // XYZW: -0.5, -0.5, -0.5, 1.0; RGB: 0.0, 0.0, 0.0
	0xc0003fff, 0x7fffc000, 0x000003e0,  // XYZW: +0.5, -0.5, -0.5, 1.0; RGB: 1.0, 0.0, 0.0
	0x3fffc000, 0x7fffc000, 0x001f0000,  // XYZW: -0.5, +0.5, -0.5, 1.0; RGB: 0.0, 1.0, 0.0
	0x3fff3fff, 0x7fffc000, 0x001f03e0,  // XYZW: +0.5, +0.5, -0.5, 1.0; RGB: 1.0, 1.0, 0.0
	0xc000c000, 0x7fff3fff, 0x78000000,  // XYZW: -0.5, -0.5, -0.5, 1.0; RGB: 0.0, 0.0, 1.0
	0xc0003fff, 0x7fff3fff, 0x780003e0,  // XYZW: +0.5, -0.5, -0.5, 1.0; RGB: 1.0, 0.0, 1.0
	0x3fffc000, 0x7fff3fff, 0x781f0000,  // XYZW: -0.5, +0.5, -0.5, 1.0; RGB: 0.0, 1.0, 1.0
	0x3fff3fff, 0x7fff3fff, 0x781f03e0   // XYZW: +0.5, +0.5, -0.5, 1.0; RGB: 1.0, 1.0, 1.0
};

static const u16 indices[]={
	0, 2, 4, 4, 2, 6,
	1, 5, 3, 3, 5, 7,
	0, 4, 1, 1, 4, 5,
	2, 3, 6, 6, 3, 7,
	0, 1, 2, 1, 3, 2,
	5, 4, 6, 7, 5, 6
};



sg_cube *sg_cube_new(const char *name)
{

	sg_cube *c = (sg_cube*)sg_entity_new(name, sizeof(sg_cube) - sizeof(sg_entity));

	gr_vertexbuffer_commit_vertices(&c->entity.vbo, verts,   sizeof(verts));
	gr_vertexbuffer_commit_indices( &c->entity.vbo, indices, sizeof(indices));

	gr_command *cube = gr_commandlist_append(&c->entity.cmds, NULL, 1, true);

	cube->shader   = gr_shader_load("shaders/cube.a");
	cube->vertices = &c->entity.vbo;
	cube->count    = 36;

	c->entity.recalculate = true;

	return c;

}



void sg_cube_del(sg_cube *c)
{

	sg_entity_del(&c->entity);

}

