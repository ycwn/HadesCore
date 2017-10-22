


import std.stdio;
import std.string;

import ecs.entity;
import ecs.component;
import ecs.component_ai;
import ecs.component_camera;
import ecs.component_dynamics;
import ecs.component_light;
import ecs.component_sprite;
import ecs.component_transform;

import hades;



void main(string[] argv)
{

	auto engine = hades_create();

	hades_parse_commandline(argv);

	gr_set_video();

	alias entity!(component_light,  component_ai, component_sprite, component_transform) enemy_willowisp;
	alias entity!(component_ai,     component_sprite, component_transform)               enemy_bandit;
	alias entity!(component_camera, component_sprite, component_transform)               player;


	auto wisp   = new enemy_willowisp("Will O' the Wisp");
	auto bandit = new enemy_bandit(   "Marauder");
	auto pc     = new player(         "player1");

	writeln("------------------------------------");
	writefln("pc.has(component_ai)        = %s\t(%x)", pc.has!component_ai(),        pc.get!component_ai());
	writefln("pc.has(component_camera)    = %s\t(%x)", pc.has!component_camera(),    pc.get!component_camera());
	writefln("pc.has(component_sprite)    = %s\t(%x)", pc.has!component_sprite(),    pc.get!component_sprite());
	writefln("pc.has(component_light)     = %s\t(%x)", pc.has!component_light(),     pc.get!component_light());
	writefln("pc.has(component_transform) = %s\t(%x)", pc.has!component_transform(), pc.get!component_transform());

	writeln("------------------------------------");
	writefln("wisp.has(component_ai)        = %s\t(%x)", wisp.has!component_ai(),        wisp.get!component_ai());
	writefln("wisp.has(component_camera)    = %s\t(%x)", wisp.has!component_camera(),    wisp.get!component_camera());
	writefln("wisp.has(component_sprite)    = %s\t(%x)", wisp.has!component_sprite(),    wisp.get!component_sprite());
	writefln("wisp.has(component_light)     = %s\t(%x)", wisp.has!component_light(),     wisp.get!component_light());
	writefln("wisp.has(component_transform) = %s\t(%x)", wisp.has!component_transform(), wisp.get!component_transform());

	writeln("------------------------------------");
	writefln("bandit.has(component_ai)        = %s\t(%x)", bandit.has!component_ai(),        bandit.get!component_ai());
	writefln("bandit.has(component_camera)    = %s\t(%x)", bandit.has!component_camera(),    bandit.get!component_camera());
	writefln("bandit.has(component_sprite)    = %s\t(%x)", bandit.has!component_sprite(),    bandit.get!component_sprite());
	writefln("bandit.has(component_light)     = %s\t(%x)", bandit.has!component_light(),     bandit.get!component_light());
	writefln("bandit.has(component_transform) = %s\t(%x)", bandit.has!component_transform(), bandit.get!component_transform());

	writefln("bandit.get(component_ai)     = %x", bandit.get!component_ai());

	log_printf(LOG_DEBUG, "SEE YOU AT THE PARTY, RICHTER!: %s", engine.buildinfo.user);


	auto scene    = sg_scenegraph_new("root");
	auto triangle = sg_triangle_new("tri");

	//sg_triangle_attach(triangle, scene);
	sg_entity_attach(triangle.entity, scene);
	sg_scenegraph_activate(scene);

	while (hades_update()) {
	}

	sg_triangle_del(triangle);
	sg_scenegraph_del(scene);

}

