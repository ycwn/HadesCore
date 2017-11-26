

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/archive.h"
#include "core/blob.h"


#define AR_MAGIC      "!<arch>\n"
#define AR_MAGIC_LEN  8
#define AR_INDEX      "//"
#define AR_INDEX_LEN  2
#define AR_MARK0      '\x60'
#define AR_MARK1      '\x0a'


typedef struct entry {

	char name[16];
	char time[12];
	char uid[6];
	char gid[6];
	char mode[8];
	char size[10];
	char mark[2];

} entry;



bool ar_init(archive *ar, const void *buf, size_t len)
{

	if (memcmp(buf, AR_MAGIC, AR_MAGIC_LEN) != 0)
		return false;

	ar->ptr = buf;
	ar->len = len - AR_MAGIC_LEN;

	ar->data  = buf + AR_MAGIC_LEN;
	ar->index = NULL;

	if (!memcmp(ar->data, AR_INDEX, AR_INDEX_LEN)) {

		int len = ar_get_length(ar, 0);

		if (len & 1)
			len++;

		ar->index  = ar->data  + sizeof(entry);
		ar->data   = ar->index + len;
		ar->len   -= len + sizeof(entry);
	}

	return true;

}



bool ar_open(archive *ar, const char *file)
{

	return ar_openfd(ar, blob_open(file, BLOB_REV_LAST));

}



bool ar_openfd(archive *ar, int id)
{

	if (id < 0)
		return false;

	return ar_init(ar, blob_get_data(id), blob_get_length(id));

}



int ar_get(archive *ar, const char *name)
{

	char buffer[512];

	for (int id=0; id != BLOB_INVALID; id = ar_get_next(ar, id))
		if (!strcmp(ar_get_name(ar, id, buffer, sizeof(buffer)), name))
			return id;

	return BLOB_INVALID;

}



int ar_find(archive *ar, int id, const char *path, int match)
{

	const int  path_len     = strlen(path);
	const int  match_mode   = match & (BLOB_MATCH_INVERT - 1);
	const bool match_invert = match & BLOB_MATCH_INVERT;

	char buffer[512];

	while (true) {

		id = ar_get_next(ar, id);

		if (id < 0)
			return BLOB_INVALID;

		const char *name = ar_get_name(ar, id, buffer, sizeof(buffer));
		const int   len  = strlen(name);

		const bool name_match =
			(match_mode == BLOB_MATCH_ANY)?      true:
			(match_mode == BLOB_MATCH_CONTAINS)? (strstr(name, path) != NULL):
			(match_mode == BLOB_MATCH_EXACT)?    (strcmp(name, path) == 0):
			(match_mode == BLOB_MATCH_PREFIX)?   (len >= path_len) && !strncmp(name, path, path_len):
			(match_mode == BLOB_MATCH_SUFFIX)?   (len >= path_len) && !strncmp(name + len - path_len, path, path_len):
			false;

		if (name_match != match_invert)
			return id;

	}

}



int ar_get_next(archive *ar, int id)
{

	if (id >= ar->len)
		return BLOB_INVALID;

	if (id < 0)
		return 0;

	const entry *ent = (const entry*)(ar->data + id);

	if (ent->mark[0] != AR_MARK0 || ent->mark[1] != AR_MARK1)
		return BLOB_INVALID;

	int len = ar_get_length(ar, id);

	if (len & 1)
		len++;

	int next = id + len + sizeof(entry);

	return (next < ar->len)? next: BLOB_INVALID;

}



const char *ar_get_name(archive *ar, int id, char *buf, size_t len)
{

	if (id < 0 || id >= ar->len)
		return "";

	const entry *ent = (const entry*)(ar->data + id);

	if (ent->mark[0] != AR_MARK0 || ent->mark[1] != AR_MARK1)
		return "";

	const char *name_start = ent->name;

	if (name_start[0] == '/' && ar->index != NULL)
		name_start = (const char*)ar->index + atoi(&name_start[1]);

	const char *name_end = strchr(name_start, '/');

	if (name_end - name_start + 1 > len)
		return "";

	memcpy(buf, name_start, name_end - name_start);

	buf[name_end - name_start] = '\0';

	return buf;

}



uint ar_get_time(archive *ar, int id)
{

	if (id < 0 || id >= ar->len)
		return 0;

	const entry *ent = (const entry*)(ar->data + id);

	if (ent->mark[0] != AR_MARK0 || ent->mark[1] != AR_MARK1)
		return 0;

	return strtoul(ent->time, NULL, 10);

}



uint ar_get_uid(archive *ar, int id)
{

	if (id < 0 || id >= ar->len)
		return 0;

	const entry *ent = (const entry*)(ar->data + id);

	if (ent->mark[0] != AR_MARK0 || ent->mark[1] != AR_MARK1)
		return 0;

	return strtoul(ent->uid, NULL, 10);

}



uint ar_get_gid(archive *ar, int id)
{

	if (id < 0 || id >= ar->len)
		return 0;

	const entry *ent = (const entry*)(ar->data + id);

	if (ent->mark[0] != AR_MARK0 || ent->mark[1] != AR_MARK1)
		return 0;

	return strtoul(ent->gid, NULL, 10);

}



uint ar_get_mode(archive *ar, int id)
{

	if (id < 0 || id >= ar->len)
		return 0;

	const entry *ent = (const entry*)(ar->data + id);

	if (ent->mark[0] != AR_MARK0 || ent->mark[1] != AR_MARK1)
		return 0;

	return strtoul(ent->mode, NULL, 8);

}



size_t ar_get_length(archive *ar, int id)
{

	if (id < 0 || id >= ar->len)
		return 0;

	const entry *ent = (const entry*)(ar->data + id);

	if (ent->mark[0] != AR_MARK0 || ent->mark[1] != AR_MARK1)
		return 0;

	return strtoul(ent->size, NULL, 10);

}



const void *ar_get_data(archive *ar, int id)
{

	if (id < 0 || id >= ar->len)
		return NULL;

	const entry *ent = (const entry*)(ar->data + id);

	if (ent->mark[0] != AR_MARK0 || ent->mark[1] != AR_MARK1)
		return NULL;

	return ar->data + id + sizeof(entry);

}

