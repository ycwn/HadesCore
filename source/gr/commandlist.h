

#ifndef GR_COMMANDLIST_H
#define GR_COMMANDLIST_H


typedef struct gr_commandlist {

	gr_vertexbuffer  *vertices;
	gr_uniformbuffer *uniforms;
	gr_texturearray  *textures;

	gr_command *commands;
	uint        capacity;
	uint        count;

} gr_commandlist;


void gr_commandlist_init(gr_commandlist *cmdl);
void gr_commandlist_del( gr_commandlist *cmdl);

bool gr_commandlist_expand(gr_commandlist *cmdl, uint capacity);
void gr_commandlist_purge( gr_commandlist *cmdl);
void gr_commandlist_clear( gr_commandlist *cmdl);
void gr_commandlist_update(gr_commandlist *cmdl);

void gr_commandlist_clone(const gr_commandlist *cmdl, gr_commandlist *other, uint capacity);

gr_command *gr_commandlist_append(gr_commandlist *cmdl, gr_shader *s, uint num, bool autoexpand);
void        gr_commandlist_remove(gr_commandlist *cmdl, gr_command *cmd);

static inline gr_command *gr_commandlist_begin(gr_commandlist *cmdl) { return &cmdl->commands[0]; }
static inline gr_command *gr_commandlist_end(  gr_commandlist *cmdl) { return &cmdl->commands[cmdl->count]; }
static inline gr_command *gr_commandlist_front(gr_commandlist *cmdl) { return &cmdl->commands[0]; }
static inline gr_command *gr_commandlist_back( gr_commandlist *cmdl) { return &cmdl->commands[cmdl->count - 1]; }

static inline const gr_command *gr_commandlist_cbegin(const gr_commandlist *cmdl) { return &cmdl->commands[0]; }
static inline const gr_command *gr_commandlist_cend(  const gr_commandlist *cmdl) { return &cmdl->commands[cmdl->count]; }
static inline const gr_command *gr_commandlist_cfront(const gr_commandlist *cmdl) { return &cmdl->commands[0]; }
static inline const gr_command *gr_commandlist_cback( const gr_commandlist *cmdl) { return &cmdl->commands[cmdl->count - 1]; }

static inline void gr_commandlist_enqueue(gr_commandlist *cmdl) {
	gr_commandqueue_enqueue(cmdl->commands, cmdl->count);
}

static inline void gr_commandlist_dequeue(gr_commandlist *cmdl) {
	gr_commandqueue_dequeue(cmdl->commands, cmdl->count);
}


#endif

