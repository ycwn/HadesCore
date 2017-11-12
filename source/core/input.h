

#ifndef __CORE_INPUT_H
#define __CORE_INPUT_H


enum {
	KEY_DOWN    = 0x03, KEY_UP       = 0x00,
	KEY_PRESSED = 0x02, KEY_RELEASED = 0x01
};

enum {
	MOUSE_DISABLED = 0,
	MOUSE_ABSOLUTE = 1 << 0,
	MOUSE_LOOK     = 1 << 1,
};


enum {
	BINDING_KEY   = 0 << 24,
	BINDING_MOUSE = 1 << 24,

	BINDING_MASK = BINDING_KEY | BINDING_MOUSE
};


typedef struct input {

	const graphics *gfx;

	float *mouse_pitch_factor;
	float *mouse_bank_factor;
	float *mouse_smoothing;
	float *mouse_leading;

	int mode;

} input;


struct keyboard {

	int key[GLFW_KEY_LAST + 1];

};


struct mouse {

	int button[16];

	float position_x, position_y;
	float velocity_x, velocity_y;

	float current_x, current_y;
	float delta_x,   delta_y;

	float x,    y;
	float bank, pitch;

};


typedef struct input_binding {

	int state;
	int code;

} input_binding;


input *input_create(const graphics *gfx);
void   input_destroy();
void   input_lock();
void   input_unlock();
void   input_reset();
void   input_mousemode(int mode);
void   input_update();

input_binding *input_binding_new(const char *name, const char *def);
void           input_binding_del(input_binding *ib);


extern struct keyboard keyboard;
extern struct mouse    mouse;


#endif

