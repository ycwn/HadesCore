

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/pixelformat.h"
#include "gr/vertexformat.h"
#include "gr/surface.h"
#include "gr/renderpass.h"
#include "gr/vertexbuffer.h"
#include "gr/uniformbuffer.h"
#include "gr/shader.h"
#include "gr/command.h"
#include "gr/commandqueue.h"
#include "gr/commandlist.h"


static gr_commandlist    cmds;
static gr_vertexbuffer   vbo;
static gr_uniformbuffer  ubo;
static int               tag;



void gr_postprocessor_create()
{

	tag = 0;

	gr_commandlist_init(&cmds);
	gr_vertexbuffer_init(&vbo);
	gr_uniformbuffer_init(&ubo);

	cmds.vertices = &vbo;
	cmds.uniforms = &ubo;

}



bool gr_postprocessor_init()
{

	static const u16 vertices[]={
		0, 0, 0, 65535, 65535, 65535, 65535, 0
	};

	static const u16 indices[]={
		0, 1, 2, 0, 2, 3
	};

	static const gpu_uniform_object u = { 0 };

	gr_vertexbuffer_commit_vertices(&vbo, vertices, sizeof(vertices));
	gr_vertexbuffer_commit_indices( &vbo, indices,  sizeof(indices));

	gr_uniformbuffer_commit(&ubo, &u, sizeof(u));

	return true;

}



void gr_postprocessor_destroy()
{

	gr_commandlist_del(&cmds);
	gr_vertexbuffer_del(&vbo);
	gr_uniformbuffer_del(&ubo);

}



int gr_postprocessor_attach(gr_shader *s)
{

	gr_command *cmd = gr_commandlist_append(&cmds, s, 1, true);

	cmd->start = 0;
	cmd->count = 6;
	cmd->tag   = tag++;

	return cmd->tag;

}



void gr_postprocessor_detach(int id)
{

	for (const gr_command *cmd=gr_commandlist_begin(&cmds); cmd != gr_commandlist_end(&cmds); cmd++)
		if (cmd->tag == id) {

			gr_commandlist_remove(&cmds, (gr_command*)cmd);
			break;

		}

}



void gr_postprocessor_commit()
{

	gr_commandlist_enqueue(&cmds);

}
