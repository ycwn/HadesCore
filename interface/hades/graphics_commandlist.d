

import hades.core_types;

import hades.graphics_command;
import hades.graphics_shader;
import hades.graphics_uniformbuffer;
import hades.graphics_vertexbuffer;


alias gr_texturearray = void; //FIXME: Not implemented

struct gr_commandlist {

	gr_vertexbuffer  *vertices;
	gr_uniformbuffer *uniforms;
	gr_texturearray  *textures;

	gr_command *commands;
	uint        capacity;
	uint        count;

}


extern(C) {

	void gr_commandlist_init(gr_commandlist *cmdl);
	void gr_commandlist_del( gr_commandlist *cmdl);

	bool gr_commandlist_expand(gr_commandlist *cmdl, uint capacity);
	void gr_commandlist_purge( gr_commandlist *cmdl);
	void gr_commandlist_clear( gr_commandlist *cmdl);
	void gr_commandlist_update(gr_commandlist *cmdl);

	void gr_commandlist_clone(const gr_commandlist *cmdl, gr_commandlist *other, uint capacity);

	gr_command *gr_commandlist_append(gr_commandlist *cmdl, gr_shader *s, uint num, bool autoexpand);
	void        gr_commandlist_remove(gr_commandlist *cmdl, gr_command *cmd);

}

const(gr_command) *gr_commandlist_begin(const gr_commandlist *cmdl) { return &cmdl.commands[0]; }
const(gr_command) *gr_commandlist_end(  const gr_commandlist *cmdl) { return &cmdl.commands[cmdl.count]; }
const(gr_command) *gr_commandlist_front(const gr_commandlist *cmdl) { return &cmdl.commands[0]; }
const(gr_command) *gr_commandlist_back( const gr_commandlist *cmdl) { return &cmdl.commands[cmdl.count - 1]; }

void gr_commandlist_enqueue(gr_commandlist *cmdl) {
	gr_commandqueue_enqueue(cmdl.commands, cmdl.count);
}

void gr_commandlist_dequeue(gr_commandlist *cmdl) {
	gr_commandqueue_dequeue(cmdl.commands, cmdl.count);
}


