

#include "core/system.h"
#include "core/types.h"
#include "core/common.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/input.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"


#define UNKNOWN "none"


typedef struct input_binding_variable {

	input_binding binding;

	int  code;
	list node;

} input_binding_variable;


static void        ibv_new(input_binding_variable *var);
static void        ibv_del(input_binding_variable *var);
static const char *ibv_get(input_binding_variable *var);
static void        ibv_set(input_binding_variable *var, const char *str);

static VARTYPE(vtbl_ibv, "KeyBinding", input_binding_variable, &ibv_new, &ibv_del, &ibv_get, &ibv_set);

static list  bindings = LIST_INIT(&bindings, NULL);
static input in;

static const char *keynames[]={

	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,      // 0
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	"space",    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  "apostrophe", // 32
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     "comma",     "minus",    "period", "slash",
	"num0",     "num1",       "num2",    "num3",      "num4",      "num5",     "num6",   "num7",
	"num8",     "num9",       UNKNOWN,   "semicolon", UNKNOWN,     "equal",    UNKNOWN,  UNKNOWN,
	UNKNOWN,    "a",          "b",       "c",         "d",         "e",        "f",      "g",          // 64
	"h",        "i",          "j",       "k",         "l",         "m",        "n",      "o",
	"p",        "q",          "r",       "s",         "t",         "u",        "v",      "w",
	"x",        "y",          "z",       "lbracket",  "backslash", "rbracket", UNKNOWN,  UNKNOWN,
	"grave",    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,      // 96
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,      // 128
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    "world1",     "world2",  UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,      // 160
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,      // 192
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,      // 224
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	"escape",   "enter",      "tab",     "backspace", "insert",    "delete",   "right",  "left",       // 256
	"down",     "up",         "pageup",  "pagedown",  "home",      "end",      UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      UNKNOWN,   UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	"capslock", "scrolllock", "numlock", "printsc",   "pause",     UNKNOWN,    UNKNOWN,  UNKNOWN,
	UNKNOWN,    UNKNOWN,      "f1",      "f2",        "f3",        "f4",       "f5",     "f6",         // 288
	"f7",       "f8",         "f9",      "f10",       "f11",       "f12",      "f13",    "f14",
	"f15",      "f16",        "f17",     "f18",       "f19",       "f20",      "f21",    "f22",
	"f23",      "f24",        "f25",     UNKNOWN,     UNKNOWN,     UNKNOWN,    UNKNOWN,  UNKNOWN,
	"kp0",      "kp1",        "kp2",     "kp3",       "kp4",       "kp5",      "kp6",    "kp7",        // 320
	"kp8",      "kp9",        "kpdec",   "kpdiv",     "kpmul",     "kpsub",    "kpadd",  "kpenter",
	"kpequal",  UNKNOWN,      UNKNOWN,   UNKNOWN,     "lshift",    "lctrl",    "lalt",   "lsuper",
	"rshift",   "rctrl",      "ralt",    "rsuper",    "menu",      UNKNOWN,    UNKNOWN,  UNKNOWN

};

struct keyboard keyboard;
struct mouse    mouse;



input *input_create(const graphics *gfx)
{

	in.gfx = gfx;

	in.mouse_pitch_factor = var_new_float("input.mouse.pitch",  "1.0");
	in.mouse_bank_factor  = var_new_float("input.mouse.bank",   "1.0");
	in.mouse_smoothing    = var_new_float("input.mouse.smooth", "0.25");
	in.mouse_leading      = var_new_float("input.mouse.lead",   "1.0");

	szero(keyboard);
	szero(mouse);

	return &in;

}



void input_destroy()
{

	var_del(in.mouse_pitch_factor);
	var_del(in.mouse_bank_factor);

}



int input_get_mousemode()
{

	return in.mode;

}



void input_set_mousemode(int mode)
{

	//if ((in.mode ^ mode) & MOUSE_LOCKED != 0)
		glfwSetInputMode(in.gfx->window, GLFW_CURSOR, (mode & MOUSE_LOCKED)? GLFW_CURSOR_DISABLED: GLFW_CURSOR_NORMAL);

	in.mode = mode;

}



void input_update()
{

	if (in.gfx == NULL || in.gfx->window == NULL)
		return;

	for (list *ib=list_begin(&bindings); ib != list_end(&bindings); ib=ib->next) {

		input_binding_variable *ibv = LIST_PTR(input_binding_variable, ib);

		if (ibv->code == GLFW_KEY_UNKNOWN)
			continue;

		int id = ibv->code & 0x0000ffff;

		if ((ibv->code & BINDING_MASK) == BINDING_KEY) {

			ibv->binding.state = (ibv->binding.state >> 1) | (glfwGetKey(in.gfx->window, id)? 2: 0);
			keyboard.key[id]   = ibv->binding.state;

		} else if ((ibv->code & BINDING_MASK) == BINDING_MOUSE) {

			ibv->binding.state = (ibv->binding.state >> 1) | (glfwGetMouseButton(in.gfx->window, id)? 2: 0);
			mouse.button[id]   = ibv->binding.state;

		}

	}

	double mouse_x;
	double mouse_y;

	glfwGetCursorPos(in.gfx->window, &mouse_x, &mouse_y);

	mouse_x /= 1024.0f;
	mouse_y /= 576.0f;

	mouse.velocity_x = (mouse_x - mouse.position_x) * *in.mouse_smoothing + mouse.velocity_x * (1.0f - *in.mouse_smoothing);
	mouse.velocity_y = (mouse_y - mouse.position_y) * *in.mouse_smoothing + mouse.velocity_y * (1.0f - *in.mouse_smoothing);
	mouse.position_x = mouse_x;
	mouse.position_y = mouse_y;

	mouse.delta_x    = mouse_x + *in.mouse_leading * mouse.velocity_x - mouse.current_x;
	mouse.delta_y    = mouse_y + *in.mouse_leading * mouse.velocity_y - mouse.current_y;
	mouse.current_x += mouse.delta_x;
	mouse.current_y += mouse.delta_y;


	if (in.mode & MOUSE_ABSOLUTE) {

		mouse.x = clampf(mouse.x + mouse.delta_x, -1.0f, +1.0f);
		mouse.y = clampf(mouse.y - mouse.delta_y, -1.0f, +1.0f);

	}

	if (in.mode & MOUSE_LOOK) {

		mouse.bank  += mouse.delta_x * *in.mouse_bank_factor;
		mouse.pitch = clampf(mouse.pitch + mouse.delta_y * *in.mouse_pitch_factor, -0.7f, +0.7f);

	}

}



input_binding *input_binding_new(const char *name, const char *def)
{

	return var_new(name, &vtbl_ibv, def);

}



void input_binding_del(input_binding *ib)
{

	var_del(ib);

}



void ibv_new(input_binding_variable *var)
{

	var->binding.state = KEY_RELEASED;
	var->binding.code  = GLFW_KEY_UNKNOWN;

	var->code = GLFW_KEY_UNKNOWN;

	list_init(&var->node, var);
	list_append(&bindings, &var->node);

}



void ibv_del(input_binding_variable *var)
{

	list_remove(&var->node);

}



const char *ibv_get(input_binding_variable *var)
{

	static char buf[32];

	if (var->code == GLFW_KEY_UNKNOWN)
		return UNKNOWN;

	int id = var->code & 0x0000ffff;

	if ((var->code & BINDING_MASK) == BINDING_KEY) {

		if (id <= GLFW_KEY_LAST)
			return keynames[id];

	} else if ((var->code & BINDING_MASK) == BINDING_MOUSE) {

		if (id >= 16)
			return UNKNOWN;

		sprintf(buf, "mouse%d", id);
		return buf;

	}

	return UNKNOWN;
}



void ibv_set(input_binding_variable *var, const char *str)
{

	if (!strcmp(str, UNKNOWN)) {

		var->code         = GLFW_KEY_UNKNOWN;
		var->binding.code = var->code;
		return;

	}

	if (strlen(str) >= 6 && !memcmp(str, "mouse", 5)) {

		int id = atoi(str + 5);

		var->code         = (id >= 0 && id < 16)? id | BINDING_MOUSE: GLFW_KEY_UNKNOWN;
		var->binding.code = var->code;
		return;

	}

	for (int n=0; n <= GLFW_KEY_LAST; n++)
		if (!strcmp(str, keynames[n])) {

			var->code         = n | BINDING_KEY;
			var->binding.code = var->code;
			return;

		}

}

