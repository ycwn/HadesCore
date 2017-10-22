

import std.string;


struct graphics;
struct sound;
struct input;

struct hades {

	private struct buildinfo_t {

		uint number;
		uint revision;
		uint timestamp;

		const char *date;
		const char *machine;
		const char *user;

	}

	buildinfo_t buildinfo;

	graphics *gfx;
	sound    *snd;
	input    *inp;

	bool terminate;

}


extern(C) {

	const(hades) *hades_create();
	void          hades_destroy();
	void          hades_parse_commandline(ref string[] argv);
	bool          hades_update();
	void          hades_fail(const char *msg, ...);

}

