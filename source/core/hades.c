

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/hades.h"
#include "core/list.h"
#include "core/blob.h"
#include "core/chrono.h"
#include "core/input.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"

#include "sg/transform.h"
#include "sg/scenegraph.h"


extern const unsigned int  _hades_build_number;
extern const unsigned int  _hades_build_revision;
extern const unsigned int  _hades_build_timestamp;
extern const char         *_hades_build_date;
extern const char         *_hades_build_machine;
extern const char         *_hades_build_user;

static hades hades_core;


static void report_glfw_error(int err, const char *desc);



const hades *hades_create()
{

	atexit(hades_destroy);

	glfwSetErrorCallback(&report_glfw_error);
	glfwInit();

	hades_core.buildinfo.number    = _hades_build_number;
	hades_core.buildinfo.revision  = _hades_build_revision;
	hades_core.buildinfo.timestamp = _hades_build_timestamp;
	hades_core.buildinfo.date      = _hades_build_date;
	hades_core.buildinfo.machine   = _hades_build_machine;
	hades_core.buildinfo.user      = _hades_build_user;

	hades_core.gfx = NULL;
	hades_core.snd = NULL;
	hades_core.in  = NULL;

	hades_core.terminate = false;

	log_create();
	log_set_writer(0, log_writer_terminal_color, NULL);

	blob_create();
	blob_load("build/data0.blob");
	blob_finalize();

	chrono_create();

	if ((hades_core.gfx = gr_create()) == NULL) {

		hades_fail("core: Failed to initialize graphics!\n");
		return NULL;

	}

	if ((hades_core.in = input_create(hades_core.gfx)) == NULL) {

		hades_fail("core: Failed to initialize input!\n");
		return NULL;

	}

	return &hades_core;

}



void hades_destroy()
{

	input_destroy();
	gr_destroy();
	chrono_destroy();
	blob_destroy();
	log_destroy();

	glfwTerminate();

}



void hades_parse_commandline(const druntime_array *argv)
{

	for (int n=0; n < argv->num; n++) {

		const druntime_string *str = druntime_array_at(argv, druntime_string, n);

		if (str->string[0] != '+')
			continue;

		const char *eq = strchr(str->string, '=');

		if (eq == NULL)
			continue;

		char var[str->length + 1];
		char val[str->length + 1];

		mzero(var);
		strncpy(var, str->string + 1, eq - str->string - 1);

		mzero(val);
		strncpy(val, eq + 1, str->length - (eq - str->string + 1));

		var_set(var, val);

	}

}



bool hades_update()
{

	hades_core.terminate = glfwWindowShouldClose(hades_core.gfx->window);

	if (hades_core.terminate)
		return false;

	glfwPollEvents();

	chrono_update(); // Advance game time
	input_update();  // Gather inputs
	// Trigger script events
	// Process UI updates
	// Simulate AI
	// Update animations
	// Simulate physics
	sg_transform_update();   // Update transform hierarchy
	sg_scenegraph_harvest(); // Harvest visible objects and populate render queue
	// Update uniform buffers
	//sg_transform_clear();    // Purge modified transforms
	gr_submit();  // Submit render queue to GPU

	return true;

}



void hades_fail(const char *msg, ...)
{
	va_list argv;
	int     size = 1024;

	while (1) {

		va_start(argv, msg);

		char buffer[size];
		int len = vsnprintf(buffer, size, msg, argv);

		va_end(argv);

		if (len < 0 || len >= size) {

			size *= 2;
			continue;

		}

		log_printf(LOG_CRITICAL, "%s", buffer);
		//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Hades Core", buffer, NULL);
		break;

	}

}



void report_glfw_error(int err, const char *desc)
{

	log_e("GLFW: %d: %s", err, desc);

}

