

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/algorithm.h"
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
#include "gr/commandqueue.h"


struct queue_t {

	gr_command **cmds;
	uint         count;
	uint         capacity;

};

static struct queue_t ready_queue;
static struct queue_t render_queue;
static struct queue_t merge_queue;


static inline int command_compare(const gr_command **a, const gr_command **b)
{

	return  ((*a)->shader->stage != (*b)->shader->stage)?
			(*a)->shader->stage - (*b)->shader->stage:
			(*a)->vertices      - (*b)->vertices;

}

GENERATE_SHELL_SORT(commandqueue_sort, gr_command*, command_compare);



void gr_commandqueue_create()
{

	ready_queue.cmds     = NULL;
	ready_queue.count    = 0;
	ready_queue.capacity = 0;

	render_queue.cmds     = NULL;
	render_queue.count    = 0;
	render_queue.capacity = 0;

	merge_queue.cmds     = NULL;
	merge_queue.count    = 0;
	merge_queue.capacity = 0;

}



void gr_commandqueue_destroy()
{

	free(ready_queue.cmds);
	free(render_queue.cmds);
	free(merge_queue.cmds);

}



void gr_commandqueue_enqueue(gr_command *cmd, uint num)
{

	gr_command *cmds[num];
	uint        count = 0;

	for (int n=0; n < num; n++) {

		if (cmd[n].state <= GR_COMMAND_STATE_INACTIVE)
			cmds[count++] = &cmd[n];

		cmd[n].state = GR_COMMAND_STATE_RENDERING;

	}

	if (count <= 0)
		return;

	if (ready_queue.count + count > ready_queue.capacity) {

		ready_queue.capacity += 1024;
		ready_queue.cmds      = realloc(ready_queue.cmds, ready_queue.capacity * sizeof(gr_command*));

	}

	memcpy(&ready_queue.cmds[ready_queue.count], cmds, count * sizeof(gr_command*));

	ready_queue.count += count;

}



void gr_commandqueue_dequeue(gr_command *cmd, uint num)
{

	for (int n=0; n < num; n++) {

		if (cmd[n].index >= 0)
			render_queue.cmds[cmd[n].index] = NULL;

		cmd[n].index = -1;

	}

}



void gr_commandqueue_consume()
{

	if (ready_queue.count > 1)
		commandqueue_sort(ready_queue.cmds, ready_queue.count);

	merge_queue.count = 0;

	if (merge_queue.capacity < render_queue.count + ready_queue.count) {

		merge_queue.capacity = 1024 * ((render_queue.count + ready_queue.count + 1023) / 1024);
		merge_queue.cmds     = realloc(merge_queue.cmds, merge_queue.capacity * sizeof(gr_command*));

	}

	gr_command **render = render_queue.cmds, **render_end = render + render_queue.count;
	gr_command **ready  = ready_queue.cmds,  **ready_end  = ready  + ready_queue.count;

	while (render < render_end && ready < ready_end) {

		if (*render == NULL) {

			render++;
			continue;

		}

		gr_command *cmd = (command_compare((const gr_command**)render, (const gr_command**)ready) < 0)?
				*render++:
				*ready++;

		if (--cmd->state <= GR_COMMAND_STATE_INACTIVE) {

			cmd->index = -1;
			continue;

		}

		cmd->index = merge_queue.count;
		merge_queue.cmds[merge_queue.count++] = cmd;

	}

	while (render < render_end) {

		gr_command *cmd = *render++;

		if (cmd == NULL)
			continue;

		if (--cmd->state <= GR_COMMAND_STATE_INACTIVE) {

			cmd->index = -1;
			continue;

		}

		cmd->index = merge_queue.count;
		merge_queue.cmds[merge_queue.count++] = cmd;

	}

	while (ready < ready_end) {

		gr_command *cmd = *ready++;

		if (--cmd->state <= GR_COMMAND_STATE_INACTIVE) {

			cmd->index = -1;
			continue;

		}

		cmd->index = merge_queue.count;
		merge_queue.cmds[merge_queue.count++] = cmd;

	}

	ready_queue.count  = 0;
	render_queue.count = 0;

	struct queue_t tmp = render_queue;

	render_queue = merge_queue;
	merge_queue  = tmp;

}



const gr_command **gr_commandqueue_begin()
{

	return (const gr_command**)render_queue.cmds;

}



const gr_command **gr_commandqueue_end()
{

	return (const gr_command**)render_queue.cmds + render_queue.count;
}

