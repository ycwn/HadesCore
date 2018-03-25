

#ifndef GR_POSTPROCESSOR_H
#define GR_POSTPROCESSOR_H


void gr_postprocessor_create();
void gr_postprocessor_destroy();

bool gr_postprocessor_init();

int  gr_postprocessor_attach(gr_shader *s);
void gr_postprocessor_detach(int id);
void gr_postprocessor_commit();


#endif

