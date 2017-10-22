

import hades.scenegraph_entity;
import hades.scenegraph_scenegraph;

import hades.graphics_vertexbuffer;


struct sg_triangle {

	sg_entity       *entity;
	gr_vertexbuffer  vbo;

}


extern(C) {

	sg_triangle *sg_triangle_new(const char *name);
	void         sg_triangle_del(sg_triangle *t);

}


void sg_triangle_attach(sg_triangle *t, sg_scenegraph *sg) { sg_entity_attach(t.entity, sg); }
void sg_triangle_detach(sg_triangle *t)                    { sg_entity_detach(t.entity); }

