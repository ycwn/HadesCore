

#include "core/system.h"
#include "core/types.h"
#include "core/dlang.h"
#include "core/hades.h"
#include "core/list.h"
#include "core/string.h"
#include "core/variable.h"

#include "gr/graphics.h"


extern const unsigned int  _hades_build_number;
extern const unsigned int  _hades_build_revision;
extern const unsigned int  _hades_build_timestamp;
extern const char         *_hades_build_date;
extern const char         *_hades_build_machine;
extern const char         *_hades_build_user;


static hades hades_core;


const hades *hades_create(const dl_array *argv)
{

	atexit(hades_destroy);

	if (SDL_Init(SDL_INIT_EVENTS) < 0) {

		fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
		return NULL;

	}/*
SDL_INIT_TIMER

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

	hades_core.gr = gr_create();

	hades_core.terminate = false;

	//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Hades Core", "YOU HAVE NO RESPECT FOR LOGIC", NULL);

	for (int n=0; n < argv->num; n++) {

		const dl_str *str = dl_array_at(argv, dl_str, n);

		printf("str.len = %d, '%.*s'\n", str->length, str->length, str->string);

	}

	gr_set_video(hades_core.gr);

	return &hades_core;

}



void hades_destroy()
{

	gr_destroy(hades_core.gr);

	SDL_Quit();

	printf("HASTA LA VISTA, BABY\n");

}



bool hades_update()
{

	SDL_Event event;

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT) {

			hades_core.terminate = true;
			break;

		}

	}

	return !hades_core.terminate;

}
