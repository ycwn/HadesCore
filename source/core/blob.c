

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/blob.h"
#include "core/logger.h"
#include "core/string.h"


#define BLOB_MAGIC  0x626f6c62

typedef struct blob_header_t {

	u32 magic;
	u32 count;
	u64 entries;
	u64 strings;

} blob_header;


typedef struct blob_entry_t {

	u64 offset;
	u64 length;
	u32 revision;
	u32 name;

} blob_entry;


typedef struct blob_t {

	int         fd;
	u64         len;
	const void *ptr;

	const blob_header *header;
	const blob_entry  *entries;
	const char        *strings;

} blob;


typedef struct index_entry_t {

	const char *name;
	const void *data;

	u64  length;
	uint revision;
	int  pack;

} index_entry;


static blob         blobs[BLOB_MAX_BLOBS];
static index_entry *entries     = NULL;
static uint         entries_num = 0;

static bool         process(int fd, const u8 *buf, u64 len);
static index_entry *find_first_revision(const char *name);
static index_entry *find_last_revision( const char *name);
static int          index_entry_cmp(const index_entry *a, const index_entry *b);



void blob_create()
{
}



void blob_destroy()
{

	free(entries);

	for (int n=0; n < BLOB_MAX_BLOBS; n++)
		if (blobs[n].ptr != NULL && blobs[n].fd >= 0) {

			munmap((void*)blobs[n].ptr, blobs[n].len);
			close(blobs[n].fd);

		}

}



bool blob_load(const char *file)
{

	int   fd  = open(file, O_RDONLY);
	off_t len = 0;
	void *ptr = NULL;

	if (fd < 0)
		goto fail;

	len = lseek(fd, 0, SEEK_END);

	if (len < 0)
		goto fail;

	ptr = mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);

	if (ptr == MAP_FAILED)
		goto fail;

	log_i("blob: Initializing memory-mapped blob from %s", file);

	if (!process(fd, ptr, len))
		goto fail;

	return true;

fail:
	log_e("blob: Can't open '%s': %d", errno);

	if (ptr != MAP_FAILED)
		munmap(ptr, len);

	if (fd  > 0)
		close(fd);

	return false;

}



bool blob_init(const void *ptr)
{

	log_i("blob: Initializing precompiled blob at %p", ptr);
	return process(-1, ptr, 0);

}



void blob_finalize()
{

	if (entries != NULL)
		return;

	entries_num = 0;

	for (int n=0; n < BLOB_MAX_BLOBS; n++)
		if (blobs[n].ptr != NULL)
			entries_num += blobs[n].header->count;

	entries = calloc(entries_num, sizeof(index_entry));

	for (int n=0, i=0; n < BLOB_MAX_BLOBS; n++) {

		const blob *b = &blobs[n];

		if (b->ptr == NULL)
			continue;

		for (int m=0; m < b->header->count; m++) {

			index_entry      *idx = &entries[i++];
			const blob_entry *e   = &b->entries[m];

			idx->name     = b->strings          + e->name;
			idx->data     = (const u8*)(b->ptr) + e->offset;
			idx->length   = e->length;
			idx->revision = e->revision;
			idx->pack     = n;

		}

	}

	qsort(entries, entries_num, sizeof(index_entry), (__compar_fn_t)&index_entry_cmp);

	for (int n=0; n < entries_num; n++)
		log_i(" %02d  %-48s  %016x  %16lld  %08x",
			entries[n].pack, entries[n].name, entries[n].data, entries[n].length, entries[n].revision);

}



int blob_open(const char *name, uint rev)
{

	if (entries == NULL || name == NULL)
		return -1;

	index_entry *first = find_first_revision(name);
	index_entry *last  = find_last_revision(name);


	if (first == last)
		return BLOB_INVALID;

	if (rev == BLOB_REV_FIRST)
		return first - entries;

	if (rev == BLOB_REV_LAST)
		return last - entries - 1;

	for (index_entry *idx=first; idx != last; ++idx)
		if (idx->revision == rev)
			return idx - entries;

	return BLOB_INVALID;

}



int blob_find(int id, const char *path, uint revision, int match, bool reverse)
{

	if (entries == NULL || path == NULL)
		return BLOB_INVALID;

	const int  path_len     = strlen(path);
	const int  match_mode   = match & (BLOB_MATCH_INVERT - 1);
	const bool match_invert = match & BLOB_MATCH_INVERT;

	while (true) {

		if (reverse) {

			id = (id < 0)? entries_num - 1: id - 1;

			if (id < 0)
				return BLOB_INVALID;

		} else {

			id++;

			if (id >= entries_num)
				return BLOB_INVALID;

		}

		const char *name = entries[id].name;
		const int   len  = strlen(name);
		const uint  rev  = entries[id].revision;

		const bool name_match =
			(match_mode == BLOB_MATCH_ANY)?      true:
			(match_mode == BLOB_MATCH_CONTAINS)? (strstr(name, path) != NULL):
			(match_mode == BLOB_MATCH_EXACT)?    (strcmp(name, path) == 0):
			(match_mode == BLOB_MATCH_PREFIX)?   (len >= path_len) && !strncmp(name, path, path_len):
			(match_mode == BLOB_MATCH_SUFFIX)?   (len >= path_len) && !strncmp(name + len - path_len, path, path_len):
			false;

		if (name_match != match_invert) {

			const char *prev = (id > 0)?               entries[id - 1].name: NULL;
			const char *next = (id < entries_num - 1)? entries[id + 1].name: NULL;

			const bool rev_match =
					(revision == BLOB_REV_ANY)?   true:
					(revision == BLOB_REV_FIRST)? (prev == NULL) || (strcmp(name, prev) != 0):
					(revision == BLOB_REV_LAST)?  (next == NULL) || (strcmp(name, next) != 0):
					(rev == revision);

			if (rev_match)
				return id;

		}

	}

}



const char *blob_get_name(int id)
{

	return (id >= 0 && id < entries_num)? entries[id].name: NULL;

}



const void *blob_get_data(int id)
{

 	return (id >= 0 && id < entries_num)? entries[id].data: NULL;

}



u64 blob_get_length(int id)
{

	return (id >= 0 && id < entries_num)? entries[id].length: 0;

}



uint blob_get_revision(int id)
{

	return (id >= 0 && id < entries_num)? entries[id].revision: 0;

}



bool process(int fd, const u8 *ptr, u64 len)
{

	if (entries != NULL || ptr == NULL)
		return false;

	blob *b = NULL;

	for (int n=0; n < BLOB_MAX_BLOBS; n++)
		if (blobs[n].ptr == NULL) {

			b = &blobs[n];
			break;

		}

	if (b == NULL)
		return false;

	b->header  = (const blob_header*)ptr;
	b->entries = (const blob_entry*)(ptr + b->header->entries);
	b->strings = (const char*)ptr        + b->header->strings;

	if (b->header->magic != BLOB_MAGIC)
		return false;

	b->fd  = fd;
	b->ptr = ptr;
	b->len = len;

	return true;

}



index_entry *find_first_revision(const char *name)
{

	index_entry *first = entries;

	for (int count=entries_num; count > 0;) { //Lower bound

		int          step = count / 2;
		index_entry *idx  = first + step;

		if (strcmp(idx->name, name) < 0) {

			first  = ++idx;
			count -= step + 1;

		} else
			count = step;

	}

	return first;

}



index_entry *find_last_revision(const char *name)
{

	index_entry *last = entries;

	for (int count=entries_num; count > 0;) { //Upper bound

		int          step = count / 2;
		index_entry *idx  = last + step;

		if (strcmp(idx->name, name) <= 0) {

			last   = ++idx;
			count -= step + 1;

		} else
			count = step;

	}

	return last;

}



int index_entry_cmp(const index_entry *a, const index_entry *b)
{

	const int namediff = strcmp(b->name, a->name);
	return  (namediff    != 0)?           namediff:
		(a->revision != b->revision)? b->revision - a->revision: b->pack - a->pack;

}

