

#ifndef __GR_GRAPHICS_H
#define __GR_GRAPHICS_H


typedef struct _graphics_t {

	SDL_Window *window;

	struct {

		variable *screen_width;
		variable *screen_height;
		variable *screen_fullscreen;

		variable *vsync_enable;
		variable *vsync_adaptive;

		variable *triple_buffer;

	} var;

	struct {
	} vk;

} graphics;


graphics *gr_create();
void      gr_destroy(graphics *self);

bool gr_set_video(        graphics *self);
void gr_request_extension(graphics *self, const char *name, bool is_dev, bool is_required);
void gr_submit(           graphics *self);


#endif


