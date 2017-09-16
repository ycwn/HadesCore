

#include "core/system.h"
#include "core/types.h"
#include "core/string.h"


#define GET_CH(buf) ((*buf)? *buf++: 0)



static char *str_alloc(string *self, size_t len)
{

	if (self->magic == CORE_STR_MAGIC) {

		if (len > self->capacity) {

			self->str      = realloc(self->str, len + 1);
			self->capacity = len;

		}

		return self->str;

	} else {

		if (len >= CORE_STR_BUFFER) {

			self->str      = strcpy(malloc(len + 1), self->buf);
			self->magic    = CORE_STR_MAGIC;
			self->capacity = len;

			return self->str;

		}

		return self->buf;

	}

}



void str_new(string *self)
{
	memset(self->buf, 0, sizeof(self->buf));

}



void str_del(string *self)
{

	if (self->magic == CORE_STR_MAGIC)
		free(self->str);

	self->magic    = 0;
	self->capacity = 0;
	self->str      = NULL;

}



const char *str_c(const string *self)
{

	return (self->magic == CORE_STR_MAGIC)? self->str: self->buf;

}



int str_i(const string *self)
{

	return atoi(str_c(self));

}



float str_f(const string *self)
{

	return atof(str_c(self));

}



size_t str_len(const string *self)
{

	return strlen(str_c(self));

}



size_t str_len8(const string *self)
{

	size_t len = 0;

	unsigned int   ch;
	unsigned int   cp;
	unsigned char *buf = (unsigned char*)str_c(self);

	while ((ch = GET_CH(buf))) {

		if      (ch < 0x80) cp = ch;
		else if (ch < 0xc2) cp = 0;
		else if (ch < 0xe0) cp = ((ch & 0x1f) <<  6) +  (GET_CH(buf) & 0x3f);
		else if (ch < 0xf0) cp = ((ch & 0x0f) << 12) + ((GET_CH(buf) & 0x3f) <<  6) +  (GET_CH(buf) & 0x3f);
		else if (ch < 0xf5) cp = ((ch & 0x07) << 18) + ((GET_CH(buf) & 0x3f) << 12) + ((GET_CH(buf) & 0x3f) << 6) + (GET_CH(buf) & 0x3f);
		else                cp = 0;

		len++;

	}

	return len;

}



void str_clear(string *self)
{

	*(char*)str_c(self) = 0;

}



void str_printf(string *self, const char *fmt, ...)
{

	va_list argv;

	size_t buffer_size = 1024;

	while (1) {

		va_start(argv, fmt);

		char buffer[buffer_size];
		int  len = vsnprintf(buffer, buffer_size, fmt, argv);

		va_end(argv);

		if (len < 0 || len >= buffer_size) {

			buffer_size *= 2;
			continue;

		}

		strcpy(str_alloc(self, len), buffer);
		break;

	}

}



void str_appendf(string *self, const char *fmt, ...)
{

	va_list argv;

	size_t buffer_size = 1024;

	while (1) {

		va_start(argv, fmt);

		char buffer[buffer_size];
		int  len = vsnprintf(buffer, buffer_size, fmt, argv);

		va_end(argv);

		if (len < 0 || len >= buffer_size) {

			buffer_size *= 2;
			continue;

		}

		strcat(str_alloc(self, str_len(self) + len), buffer);
		break;

	}

}



void str_dup_s(string *self, const string *s)
{

	strcpy(str_alloc(self, str_len(s)), str_c(s));

}



void str_dup_c(string *self, const char *c)
{

	strcpy(str_alloc(self, strlen(c)), c);

}



void str_cat_s(string *self, const string *s)
{

	strcat(str_alloc(self, str_len(self) + str_len(s)), str_c(s));

}



void str_cat_c(string *self, const char *c)
{

	strcat(str_alloc(self, str_len(self) + strlen(c)), c);

}



int str_find_s(const string *self, const string *s)
{

	const char *base = str_c(self);
	const char *str  = strstr(base, str_c(s));

	if (str != NULL)
		return str - base;

	else
		return -1;

}



int str_find_c(const string *self, const char *c)
{

	const char *base = str_c(self);
	const char *str  = strstr(base, c);

	if (str != NULL)
		return str - base;

	else
		return -1;

}



int str_index(const string *self, int ch)
{

	const char *base = str_c(self);
	const char *str  = strchr(base, ch);

	if (str != NULL)
		return str - base;

	else
		return -1;

}



int str_rindex(const string *self, int ch)
{

	const char *base = str_c(self);
	const char *str  = strrchr(base, ch);

	if (str != NULL)
		return str - base;

	else
		return -1;

}

