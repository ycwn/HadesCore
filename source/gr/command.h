

#ifndef GR_COMMAND_H
#define GR_COMMAND_H


enum {
	GR_COMMAND_STATE_INACTIVE,
	GR_COMMAND_STATE_PENDING,
	GR_COMMAND_STATE_RENDERING
};

typedef void gr_texturearray;

typedef struct gr_command {

	gr_shader        *shader;
	gr_vertexbuffer  *vertices;
	gr_uniformbuffer *uniforms;
	gr_uniformbuffer *material;
	gr_texturearray  *textures;

	int state;
	int index;
	int start;
	int count;
	int tag;

} gr_command;


void gr_command_create();
void gr_command_destroy();

void gr_command_init( gr_command  *cmds, uint  num);
void gr_command_alloc(gr_command **cmds, uint *num);
void gr_command_free( gr_command **cmds, uint *num);

void gr_command_reserve(uint count, uint capacity);
void gr_command_purge();


#endif

