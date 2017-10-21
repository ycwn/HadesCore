

import io = std.stdio;
import std.string;

extern(C) void hades_create(ref string[]);
extern(C) bool hades_update();



void main(string[] argv)
{

	hades_create(argv);

	while (hades_update()) {
	}

}

