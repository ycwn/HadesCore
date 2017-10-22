

module ecs.component_camera;

import ecs.component;
import ecs.component_transform;
import ecs.entity;


struct component_camera {

	component            base = component_id.CAMERA;
	component_transform *xform;

	float[16] projection;

	alias base this;


	void init(T...)(entity!T ent) {

		static assert(ent.has!component_transform());

	}

}

