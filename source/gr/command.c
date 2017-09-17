

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
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


static list blocks[GR_COMMANDS_MAX];



void gr_command_create()
{

	for (int n=0; n < GR_COMMANDS_MAX; n++)
		list_init(&blocks[n], NULL);

}



void gr_command_destroy()
{

	gr_command_purge();

}



void gr_command_init(gr_command *cmds, uint num)
{

	for (int n=0; n < num; n++) {

		cmds[n].shader   = NULL;
		cmds[n].vertices = NULL;
		cmds[n].uniforms = NULL;
		cmds[n].textures = NULL;

		cmds[n].model = NULL;

		cmds[n].state = GR_COMMAND_STATE_INACTIVE;
		cmds[n].index = -1;
		cmds[n].start = 0;
		cmds[n].count = 0;

	}

}



void gr_command_alloc(gr_command **cmds, uint *num)
{

	if (cmds == NULL || num == NULL || *num < 1)
		return;

	for (int n=*num; n < GR_COMMANDS_MAX; n++)
		if (!list_empty(&blocks[n])) {

			*cmds = LIST_PTR(gr_command, list_pop(&blocks[n]));
			*num  = n;
			gr_command_init(*cmds, n);
			return;

		}

	*cmds = malloc(sizeof(gr_command) * *num);

	gr_command_init(*cmds, *num);

}



void gr_command_free(gr_command **cmds, uint *num)
{

	if (cmds == NULL || num == NULL || *cmds == NULL || *num < 1)
		return;

	if (*num < GR_COMMANDS_MAX) {

		list *blk = (list*)*cmds;

		list_init(blk, blk);
		list_append(&blocks[*num], blk);

	} else
		free(*cmds);

	*cmds = NULL;
	*num  = 0;

}



void gr_command_reserve(uint count, uint capacity)
{

	if (count < 1 || capacity < 1 || capacity >= GR_COMMANDS_MAX)
		return;

	for (int n=0; n < count; n++) {

		list *blk = malloc(sizeof(gr_command) * capacity);

		list_init(blk, blk);
		list_append(&blocks[capacity], blk);

	}

}



void gr_command_purge()
{

	for (int n=0; n < GR_COMMANDS_MAX; n++)
		while (!list_empty(&blocks[n]))
			free(list_pop(&blocks[n]));

}

