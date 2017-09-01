

#ifndef __CORE_HADES_H
#define __CORE_HADES_H


typedef struct _graphics_t graphics;
typedef struct _sound_t    sound;
typedef struct _input_t    input;

typedef struct {

	struct {

		uint number;
		uint revision;
		uint timestamp;

		const char *date;
		const char *machine;
		const char *user;

	} buildinfo;

	graphics *gr;
	sound    *snd;
	input    *in;

	bool terminate;

} hades;


const hades *hades_create(const dl_array *argv);
void         hades_destroy();
bool         hades_update();



#endif

