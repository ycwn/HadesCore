

module ecs.entity;

import std.meta;
import std.string;

import ecs.component;



class entity(T...) {

	private AliasSeq!T components;
	const   string     name;


/*	static foreach(_t; T) {
		static assert(is(_t: component));
	}*/


	this(string _name) {

		name = _name;

		foreach(comp; components)
			comp.init(this);

	}


	static bool has(C)() {
		/*static*/ foreach(_t; T)
			static if (is(_t: C))
				return true;
		return false;
	}


	C *get(C)() {
		/*static*/ foreach(idx, _t; T)
			static if (is(_t: C))
				return &components[idx];
		return null;
	}

}

