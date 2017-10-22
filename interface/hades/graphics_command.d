

import hades.core_types;

import hades.graphics_shader;
import hades.graphics_vertexbuffer;
import hades.graphics_uniformbuffer;


enum {
	GR_COMMAND_STATE_INACTIVE,
	GR_COMMAND_STATE_PENDING,
	GR_COMMAND_STATE_RENDERING
}


struct gr_command {

	gr_shader        *shader;
	gr_vertexbuffer  *vertices;
	gr_uniformbuffer *uniforms;
	void             *textures;

	mat4 *model;

	int state;
	int index;
	int start;
	int count;

}


extern(C) {

	void gr_command_create();
	void gr_command_destroy();

	void gr_command_init( gr_command  *cmds, uint  num);
	void gr_command_alloc(gr_command **cmds, uint *num);
	void gr_command_free( gr_command **cmds, uint *num);

	void gr_command_reserve(uint count, uint capacity);
	void gr_command_purge();

}

