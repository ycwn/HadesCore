

module ecs.component_ai;

import ecs.component;
import ecs.entity;


struct component_ai {

	component base = component_id.AI;
	int       behavior;

	alias base this;


	void init(T...)(entity!T ent) {

	}

}

