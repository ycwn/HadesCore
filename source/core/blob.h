

#ifndef __CORE_BLOB_H
#define __CORE_BLOB_H


#define BLOB_MAX_BLOBS  1024
#define BLOB_INVALID    (-1)

enum {
	BLOB_REV_MIN   = 0,
	BLOB_REV_MAX   = UINT32_MAX,
	BLOB_REV_ANY   = BLOB_REV_MAX - 2,
	BLOB_REV_FIRST = BLOB_REV_MAX - 1,
	BLOB_REV_LAST  = BLOB_REV_MAX - 0
};

enum {
	BLOB_MATCH_ANY      = 0,
	BLOB_MATCH_EXACT    = 1,
	BLOB_MATCH_PREFIX   = 2,
	BLOB_MATCH_SUFFIX   = 3,
	BLOB_MATCH_CONTAINS = 4,
	BLOB_MATCH_INVERT   = 8
};


void blob_create();
void blob_destroy();

bool blob_load(const char *file);
bool blob_init(const void *ptr);
void blob_finalize();

int blob_open(const char *name, uint rev);
int blob_find(int id, const char *path, uint revision, int match, bool reverse);

const char *blob_get_name(    int id);
const void *blob_get_data(    int id);
u64         blob_get_length(  int id);
uint        blob_get_revision(int id);


#endif

