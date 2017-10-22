

module ecs.component_light;

import ecs.component;
import ecs.component_transform;
import ecs.entity;


struct component_light {

	component            base = component_id.LIGHT;
	component_transform *xform;

	float[4] color;
	float    radius;

	alias base this;


	void init(T...)(entity!T ent) {

		static assert(ent.has!component_transform());

	}

}

