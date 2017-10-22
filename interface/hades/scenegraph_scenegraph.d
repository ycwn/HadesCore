

import hades.core_list;

import hades.scenegraph_camera;


struct sg_scenegraph {

	list cameras;
	list entities;
	list lights;

	sg_camera *camera_active;

	list           node;
	const(char)[0] name;

}


extern(C) {

	sg_scenegraph *sg_scenegraph_new(const char *name);
	void           sg_scenegraph_del(sg_scenegraph *sg);

	sg_scenegraph *sg_scenegraph_find(const char *name);
	void           sg_scenegraph_harvest();

	void           sg_scenegraph_activate(sg_scenegraph *sg);
	bool           sg_scenegraph_is_active(sg_scenegraph *sg);
	sg_scenegraph *sg_scenegraph_get_active();

}

