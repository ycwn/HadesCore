

struct archive {

	const ubyte *ptr;
	size_t      len;

	const ubyte *data;
	const ubyte *index;

}


extern(C) {

	bool ar_init(  archive *ar, const ubyte *buf, size_t len);
	bool ar_open(  archive *ar, const char *file);
	bool ar_openfd(archive *ar, int id);

	int ar_get( archive *ar, const char *name);
	int ar_find(archive *ar, int id, const char *name, int match);

	int           ar_get_next(  archive *ar, int id);
	const(char)  *ar_get_name(  archive *ar, int id, char *buf, size_t len);
	uint          ar_get_time(  archive *ar, int id);
	uint          ar_get_uid(   archive *ar, int id);
	uint          ar_get_gid(   archive *ar, int id);
	uint          ar_get_mode(  archive *ar, int id);
	size_t        ar_get_length(archive *ar, int id);
	const(ubyte) *ar_get_data(  archive *ar, int id);

}

