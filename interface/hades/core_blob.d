

enum BLOB_MAX_BLOBS = 1024;
enum BLOB_INVALID   = -1;

enum {
	BLOB_REV_MIN   = 0,
	BLOB_REV_MAX   = uint.max,
	BLOB_REV_ANY   = BLOB_REV_MAX - 2,
	BLOB_REV_FIRST = BLOB_REV_MAX - 1,
	BLOB_REV_LAST  = BLOB_REV_MAX - 0
}

enum {
	BLOB_MATCH_ANY      = 0,
	BLOB_MATCH_EXACT    = 1,
	BLOB_MATCH_PREFIX   = 2,
	BLOB_MATCH_SUFFIX   = 3,
	BLOB_MATCH_CONTAINS = 4,
	BLOB_MATCH_INVERT   = 8
}


extern(C) {

	void blob_create();
	void blob_destroy();

	bool blob_load(const char  *file);
	bool blob_init(const ubyte *ptr);
	void blob_finalize();

	int blob_open(const char *name, uint rev);
	int blob_find(int id, const char *path, uint revision, int match, bool reverse);

	const(char)  *blob_get_name(    int id);
	const(ubyte) *blob_get_data(    int id);
	size_t        blob_get_length(  int id);
	uint          blob_get_revision(int id);

}

