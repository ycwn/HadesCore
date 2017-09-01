

#include "core/system.h"
#include "core/debug.h"
#include "core/types.h"
#include "core/list.h"
#include "core/string.h"
#include "core/variable.h"

#include "gr/graphics.h"


static bool gr_init( graphics *gr);
static void gr_reset(graphics *gr);



graphics *gr_create()
{

	if (SDL_InitSubSystem(SDL_INIT_VIDEO))
		return NULL;

	graphics *gr = malloc(sizeof(graphics));

	gr->var.screen_width      = var_new("gr.screen.width",   "1024");
	gr->var.screen_height     = var_new("gr.screen.height",   "576");
	gr->var.screen_fullscreen = var_new("gr.screen.full",       "0");
	gr->var.vsync_enable      = var_new("gr.vsync.enable",      "1");
	gr->var.vsync_adaptive    = var_new("gr.vsync.adaptive",    "1");
	gr->var.triple_buffer     = var_new("gr.tribuffer",         "0");

	return gr;

}



void gr_destroy(graphics *gr)
{

	if (gr == NULL)
		return;

	gr_reset(gr);

	var_del(gr->var.screen_width);
	var_del(gr->var.screen_height);
	var_del(gr->var.screen_fullscreen);
	var_del(gr->var.vsync_enable);
	var_del(gr->var.vsync_adaptive);
	var_del(gr->var.triple_buffer);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);

}



bool gr_set_video(graphics *gr)
{

	gr_reset(gr);

	gr->window = SDL_CreateWindow(
		"Hades Core",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		gr->var.screen_width->integer,
		gr->var.screen_height->integer,
		(gr->var.screen_fullscreen->integer)? SDL_WINDOW_FULLSCREEN: 0);

	if (gr->window == NULL)
		return false;

	return true;

}



bool gr_init(graphics *gr)
{

	return true;

}



void gr_reset(graphics *gr)
{

	if (gr->window != NULL)
		SDL_DestroyWindow(gr->window);

	gr->window = NULL;

}

