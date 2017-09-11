

#ifndef GR_FRAMEBUFFER_H
#define GR_FRAMEBUFFER_H


typedef struct gr_framebuffer_t {


} gr_framebuffer;


void gr_framebuffer_create(graphics *gr);
void gr_framebuffer_destroy();

bool gr_framebuffer_init();
void gr_framebuffer_update();


#endif

