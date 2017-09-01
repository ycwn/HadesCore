

#ifndef __CORE_STRING_H
#define __CORE_STRING_H


#define CORE_STR_BUFFER  16
#define CORE_STR_MAGIC   0xdeadbeef


typedef struct _string_t {

	union {

		char buf[CORE_STR_BUFFER];

		struct {
			int   magic;
			int   capacity;
			char *str;
		};

	};

} string;


void str_new(string *self);
void str_del(string *self);

const char *str_c(const string *self);
int         str_i(const string *self);
float       str_f(const string *self);

size_t str_len( const string *self);
size_t str_len8(const string *self);

void str_clear(  string *self);
void str_printf( string *self, const char *fmt, ...);
void str_appendf(string *self, const char *fmt, ...);

void str_dup_s(string *self, const string *s);
void str_dup_c(string *self, const char   *c);
void str_cat_s(string *self, const string *s);
void str_cat_c(string *self, const char   *c);

int str_find_s(const string *self, const string *s);
int str_find_c(const string *self, const char   *c);
int str_index( const string *self, int ch);
int str_rindex(const string *self, int ch);


#define CORE_STR_GENERIC(f, x)            \
	_Generic((x),                     \
		const char*:   str_##f##_c, \
		char*:         str_##f##_c, \
		const string*: str_##f##_s, \
		string*:       str_##f##_s)

#define str_dup( self, s) CORE_STR_GENERIC(dup,  (s))((self), (s))
#define str_cat( self, s) CORE_STR_GENERIC(cat,  (s))((self), (s))
#define str_find(self, s) CORE_STR_GENERIC(find, (s))((self), (s))


#endif


