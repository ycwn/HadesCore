

module ecs.component_sprite;

import ecs.component;
import ecs.component_transform;
import ecs.entity;


struct component_sprite {

	component            base = component_id.SPRITE;
	component_transform *xform;

	alias base this;


	void init(T...)(entity!T ent) {

		static assert(ent.has!component_transform());

	}

}

