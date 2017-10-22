

module ecs.component_transform;

import ecs.component;
import ecs.entity;


struct component_transform {

	component base = component_id.TRANSFORM;

	alias base this;


	void init(T...)(entity!T ent) {

	}

}

