

import hades.core_list;
import hades.core_types;

import hades.scenegraph_scenegraph;
import hades.scenegraph_transform;


struct sg_light {

	vec4  color;
	float radius;

	sg_transform  *transform;
	sg_scenegraph *scenegraph;

	list node;
	list node_sg;

	const(char)[0] name;

}


extern(C) {

	sg_light *sg_light_new(const char *name);
	void      sg_light_del(sg_light *l);

	sg_light *sg_light_find(const char *name);

	void sg_light_attach(sg_light *l, sg_scenegraph *sg);
	void sg_light_detach(sg_light *l);

}

