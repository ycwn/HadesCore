

import hades.core_list;

import hades.graphics_commandlist;

import hades.scenegraph_scenegraph;
import hades.scenegraph_transform;


struct sg_entity {

	gr_commandlist cmds;

	sg_transform  *transform;
	sg_scenegraph *scenegraph;

	list node;
	list node_sg;

	const(char)[0] name;

}


extern(C) {

	sg_entity *sg_entity_new(const char *name);
	void       sg_entity_del(sg_entity *e);

	sg_entity *sg_entity_find(const char *name);

	void sg_entity_attach(sg_entity *e, sg_scenegraph *sg);
	void sg_entity_detach(sg_entity *e);

}

