

#ifndef CORE_HADES_H
#define CORE_HADES_H


typedef struct {

	struct {

		uint number;
		uint revision;
		uint timestamp;

		const char *date;
		const char *machine;
		const char *user;

	} buildinfo;

	graphics *gfx;
	sound    *snd;
	input    *in;

	bool terminate;

} hades;


const hades *hades_create();
void         hades_destroy();
void         hades_parse_commandline(const druntime_array *argv);
bool         hades_update();
void         hades_fail(const char *msg, ...);


#endif

