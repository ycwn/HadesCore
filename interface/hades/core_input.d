

import hades.graphics_graphics;


enum {
	GLFW_KEY_LAST = 348
}


enum {
	KEY_DOWN    = 0x03, KEY_UP       = 0x00,
	KEY_PRESSED = 0x02, KEY_RELEASED = 0x01
}

enum {
	MOUSE_DISABLED = 0,
	MOUSE_ABSOLUTE = 1 << 0,
	MOUSE_LOOK     = 1 << 1,
	MOUSE_LOCKED   = 1 << 2
}


enum {
	BINDING_KEY   = 0 << 24,
	BINDING_MOUSE = 1 << 24,

	BINDING_MASK = BINDING_KEY | BINDING_MOUSE
}


struct input {

	const graphics *gfx;

	float *mouse_pitch_factor;
	float *mouse_bank_factor;
	float *mouse_smoothing;
	float *mouse_leading;

	int mode;

}


struct keyboard_t {

	int[GLFW_KEY_LAST + 1] key;

}


struct mouse_t {

	int[16] button;

	float position_x, position_y;
	float velocity_x, velocity_y;

	float current_x, current_y;
	float delta_x,   delta_y;

	float x,    y;
	float bank, pitch;

}


struct input_binding {

	int state;
	int code;

}


extern(C) {

	input *input_create(const(graphics) *gfx);
	void   input_destroy();
	int    input_get_mousemode();
	void   input_set_mousemode(int mode);
	void   input_update();

	input_binding *input_binding_new(const(char) *name, const(char) *def);
	void           input_binding_del(input_binding *ib);


	extern __gshared keyboard_t keyboard;
	extern __gshared mouse_t    mouse;

}

