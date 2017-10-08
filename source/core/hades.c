

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/dlang.h"
#include "core/hades.h"
#include "core/list.h"
#include "core/blob.h"
#include "core/chrono.h"
#include "core/logger.h"
#include "core/string.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"

#include "sg/transform.h"


extern const unsigned int  _hades_build_number;
extern const unsigned int  _hades_build_revision;
extern const unsigned int  _hades_build_timestamp;
extern const char         *_hades_build_date;
extern const char         *_hades_build_machine;
extern const char         *_hades_build_user;

static hades hades_core;


static void process_environment();
static void process_command_line(const dl_array *argv);



const hades *hades_create(const dl_array *argv)
{

	atexit(hades_destroy);

	if (SDL_Init(SDL_INIT_EVENTS) < 0)
		return hades_fail("SDL initialization failed: %s\n", SDL_GetError());
/*
SDL_INIT_AUDIO

SDL_INIT_JOYSTICK
SDL_INIT_HAPTIC
SDL_INIT_GAMECONTROLLER
*/
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

	if ((hades_core.gfx = gr_create()) == NULL)
		return hades_fail("core: Failed to initialize graphics!\n%s", SDL_GetError());

	process_environment();
	process_command_line(argv);

	gr_set_video(hades_core.gfx);

	return &hades_core;

}



void hades_destroy()
{

	gr_destroy();

	chrono_destroy();
	blob_destroy();
	log_destroy();

	SDL_Quit();

}



bool hades_update()
{

	SDL_Event event;

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT) {

			hades_core.terminate = true;
			break;

		}

		chrono_update();
		//input update
		//gui update
		//logic update
		sg_transform_update();

		gr_submit();

	}

	return !hades_core.terminate;

}



const void *hades_fail(const char *msg, ...)
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
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Hades Core", buffer, NULL);
		break;

	}

	return NULL;

}



void process_environment()
{

	extern char **environ;

	for (int n=0; environ[n] != NULL; n++) {

		if (strncmp(environ[n], "hades_", 6))
			continue;

		char *ptr = environ[n] + 6;
		int   len = strlen(ptr);
		char  var[len + 1];

		for (int m=0; m < len; m++)
			var[m] = (ptr[m] == '_')? '.': ptr[m];

		var[len] = '\0';

		var_set(var, SDL_getenv(environ[n]));

	}

}



void process_command_line(const dl_array *argv)
{

	for (int n=0; n < argv->num; n++) {

		const dl_str *str = dl_array_at(argv, dl_str, n);

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

