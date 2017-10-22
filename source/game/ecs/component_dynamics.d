

module ecs.component_dynamics;

import ecs.component;
import ecs.component_transform;
import ecs.entity;


struct component_dynamics {

	component            base = component_id.DYNAMICS;
	component_transform *xform;

	alias base this;


	void init(T...)(entity!T ent) {

		static assert(ent.has!component_transform());

	}


	void update()
	{
	}

}

