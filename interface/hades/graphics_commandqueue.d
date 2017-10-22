

import hades.graphics_command;


extern(C) {

	void gr_commandqueue_create();
	void gr_commandqueue_destroy();

	void gr_commandqueue_enqueue(gr_command *cmd, uint num);
	void gr_commandqueue_dequeue(gr_command *cmd, uint num);
	void gr_commandqueue_consume();

	const(gr_command) **gr_commandqueue_begin();
	const(gr_command) **gr_commandqueue_end();

}

