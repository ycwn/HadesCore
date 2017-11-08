

import hades.scenegraph_entity;
import hades.scenegraph_scenegraph;

import hades.graphics_vertexbuffer;


struct sg_cube {

	sg_entity entity;

}


extern(C) {

	sg_cube *sg_cube_new(const char *name);
	void     sg_cube_del(sg_cube *c);

}


void sg_cube_attach(sg_cube *c, sg_scenegraph *sg) { sg_entity_attach(&c.entity, sg); }
void sg_cube_detach(sg_cube *c)                    { sg_entity_detach(&c.entity); }

