

import hades.core_list;

import hades.graphics_commandlist;
import hades.graphics_uniformbuffer;
import hades.graphics_vertexbuffer;

import hades.scenegraph_scenegraph;
import hades.scenegraph_transform;


struct sg_entity {

	gr_commandlist   cmds;
	gr_vertexbuffer  vbo;
	gr_uniformbuffer ubo;

	sg_transform  *transform;
	sg_scenegraph *scenegraph;

	list node;
	list node_sg;

	bool recalculate;

	const(char)* name;

	byte[0] extra;

}


extern(C) {

	sg_entity *sg_entity_new(const char *name, size_t extra);
	void       sg_entity_del(sg_entity *e);

	sg_entity *sg_entity_find(const char *name);

	void sg_entity_attach(sg_entity *e, sg_scenegraph *sg);
	void sg_entity_detach(sg_entity *e);

}

