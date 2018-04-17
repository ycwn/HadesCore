


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

	gr_builder_define("SCREEN_WIDTH",  1024);
	gr_builder_define("SCREEN_HEIGHT", 576);

	gr_builder_add_include_path("shaders");

	if (!gr_builder_parse("shaders/renderer.conf"))
		return;

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

	writefln("bandit.get(component_ai) = %x", bandit.get!component_ai());

	log_printf(LOG_DEBUG, "SEE YOU AT THE PARTY, RICHTER!: %s", engine.buildinfo.user);

	gr_postprocessor_attach(gr_shader_find("blurh"));
	gr_postprocessor_attach(gr_shader_find("blurhv"));
	gr_postprocessor_attach(gr_shader_find("scanline"));

	auto scene   = sg_scenegraph_new("root");
	auto camera  = sg_camera_new("eye");
	auto suzanne = sg_geometry_new("Suzanne");
	auto R       = sg_transform_new("cube");

	auto nehe_r = gr_surface_open("textures/NeHe-R.dds");
	auto nehe_g = gr_surface_open("textures/NeHe-G.dds");
	auto nehe_b = gr_surface_open("textures/NeHe-B.dds");

	gr_surface_bind(nehe_b);

	auto ukey = input_binding_new("ukey", "up");
	auto dkey = input_binding_new("dkey", "down");
	auto lkey = input_binding_new("lkey", "left");
	auto quit = input_binding_new("input.key.quit", "escape");

	sg_geometry_load(suzanne, "models/cube.geo");

	//sg_camera_ortho(camera, 2.0f, 2.0f, -1.0f, +1.0f);
	sg_camera_perspective(camera, 90.0f, 0.001f, 100.0f);
	sg_camera_attach(camera, scene);
	sg_camera_activate(camera);

	suzanne.entity.transform = R;
//	camera.transform = R;

	sg_entity_attach(&suzanne.entity, scene);

	sg_scenegraph_activate(scene);

	sg_transform_translate(R, [ 0.0f, 0.0f, -2.0f, 0.0f ]);

	float x = 0.0f;
	uint  n = 0;

	while (hades_update()) {

		if (quit.state == KEY_PRESSED)
			break;

		sg_transform_rotate_angle(R, x, x * 0.9f);
		suzanne.entity.recalculate = true;
		x += 0.02f;

		if (ukey.state == KEY_PRESSED) { gr_surface_unbind(nehe_r); gr_surface_unbind(nehe_g); gr_surface_bind(nehe_b); }
		if (dkey.state == KEY_PRESSED) { gr_surface_unbind(nehe_g); gr_surface_unbind(nehe_b); gr_surface_bind(nehe_r); }
		if (lkey.state == KEY_PRESSED) { gr_surface_unbind(nehe_b); gr_surface_unbind(nehe_r); gr_surface_bind(nehe_g); }

	}

	gr_flush();

	sg_geometry_del(suzanne);
	sg_camera_del(camera);
	sg_scenegraph_del(scene);

}

