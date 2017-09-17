

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/blob.h"
#include "core/logger.h"
#include "core/archive.h"
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


static gr_vertexbuffer  guard_vertices = { 0 };
//static gr_uniformbuffer guard_uniforms;
//static gr_texturearray  guard_textures;



void gr_commandlist_init(gr_commandlist *cmdl)
{

	cmdl->modelview = -1;//mat4::Identity(); //FIXME: No vector math!

	cmdl->vertices = &guard_vertices;
	cmdl->uniforms = NULL;//&guard_uniforms; //FIXME: No uniform buffers!
	cmdl->textures = NULL;//&guard_textures; //FIXME: No textures!

	cmdl->commands = NULL;
	cmdl->capacity = 0;
	cmdl->count    = 0;

}



void gr_commandlist_del(gr_commandlist *cmdl)
{

	gr_commandlist_purge(cmdl);

}



bool gr_commandlist_expand(gr_commandlist *cmdl, uint capacity)
{

	gr_command *cmds = NULL;
	uint        num  = cmdl->capacity + capacity;

	gr_command_alloc(&cmds, &num);

	if (cmds == NULL)
		return false;

	memcpy(cmds, cmdl->commands, sizeof(gr_command) * cmdl->count);

	gr_command_free(&cmdl->commands, &cmdl->capacity);

	cmdl->commands = cmds;
	cmdl->capacity = num;

	return true;

}



void gr_commandlist_purge(gr_commandlist *cmdl)
{

	gr_commandlist_clear(cmdl);
	gr_command_free(&cmdl->commands, &cmdl->capacity);

}



void gr_commandlist_clear(gr_commandlist *cmdl)
{

	gr_commandlist_dequeue(cmdl);

	cmdl->count = 0;

}



void gr_commandlist_update(gr_commandlist *cmdl)
{

	if (cmdl->vertices == NULL)
		cmdl->vertices = &guard_vertices;

/*	if (cmdl->uniforms == NULL)
		cmdl->uniforms = &guard_uniforms;

	if (cmdl->textures == NULL)
		cmdl->textures = &guard_textures;*/


	for (int n=0; n < cmdl->count; n++) {

		gr_command *cmd = &cmdl->commands[n];

		cmd->vertices = cmdl->vertices;
		cmd->uniforms = cmdl->uniforms;
		cmd->textures = cmdl->textures;

	}

}



void gr_commandlist_clone(const gr_commandlist *cmdl, gr_commandlist *other, uint capacity)
{

	gr_commandlist_clear(other);

	if (cmdl->capacity + capacity > other->capacity)
		gr_commandlist_expand(other, cmdl->capacity + capacity - other->capacity);

	other->vertices = cmdl->vertices;
	other->uniforms = cmdl->uniforms;
	other->textures = cmdl->textures;

	for (int n=0; n < cmdl->count; n++) {

		gr_command *cmdp = &cmdl->commands[n];
		gr_command *cmdq = gr_commandlist_append(other, cmdp->shader, 1, false);

		cmdq->start = cmdp->start;
		cmdq->count = cmdp->count;

	}

}



gr_command *gr_commandlist_append(gr_commandlist *cmdl, gr_shader *s, uint num, bool autoexpand)
{

	if (num < 1)
		return NULL;

	if (cmdl->count + num > cmdl->capacity) {

		if (!autoexpand || !gr_commandlist_expand(cmdl, num))
			return NULL;

	}

	gr_command *cmd = &cmdl->commands[cmdl->count];

	cmdl->count += num;

	for (int n=0; n < num; n++) {

		cmd[n].state = GR_COMMAND_STATE_INACTIVE;
		cmd[n].index = -1;

		cmd[n].shader   = s;
		cmd[n].vertices = cmdl->vertices;
		cmd[n].uniforms = cmdl->uniforms;
		cmd[n].textures = cmdl->textures;
		cmd[n].model    = &cmdl->modelview;

	}

	return cmd;

}



void gr_commandlist_remove(gr_commandlist *cmdl, gr_command *cmd)
{

	if (cmd < cmdl->commands || cmd >= cmdl->commands + cmdl->count)
		return;

	gr_commandqueue_dequeue(cmd, 1);

	*cmd = cmdl->commands[cmdl->count - 1];
	cmdl->count--;

}

