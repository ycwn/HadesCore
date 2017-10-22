

module ecs.component;


enum component_id {

	AI        = 0,
	CAMERA    = 1,
	DYNAMICS  = 2,
	LIGHT     = 3,
	SPRITE    = 4,
	TRANSFORM = 5

}


struct component {

	@disable this();

	this(component_id _id) {
		id = _id;
	}

	const(component_id) id;

}

