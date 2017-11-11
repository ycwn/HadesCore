

#include "core/system.h"
#include "core/types.h"
#include "core/hades.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/variable.h"


static log_writer *log_writers[LOG_MAX_SLOTS] = { NULL };
static int         log_levels[LOG_MAX_SLOTS]  = { LOG_DEBUG };


static void terminal_plain_create( log_writer *self, const void *arg);
static void terminal_plain_destroy(log_writer *self);
static void terminal_plain_write(  log_writer *self, int level, const char *buf, int len);

static struct _log_terminal_plain_t {

	log_writer writer;

} log_terminal_plain = {

	.writer = {
		.create  = (log_create_t)&terminal_plain_create,
		.destroy = (log_destroy_t)&terminal_plain_destroy,
		.write   = (log_write_t)&terminal_plain_write
	}

};

log_writer *log_writer_terminal_plain = &log_terminal_plain.writer;


static void terminal_color_create( log_writer *self, const void *arg);
static void terminal_color_destroy(log_writer *self);
static void terminal_color_write(  log_writer *self, int level, const char *buf, int len);

static struct _log_terminal_color_t {

	log_writer writer;

	const char *colors[5];
	const char *color_def;

} log_terminal_color = {

	.writer = {
		.create  = (log_create_t)&terminal_color_create,
 		.destroy = (log_destroy_t)&terminal_color_destroy,
		.write   = (log_write_t)&terminal_color_write
	},

	.colors = {
		"\033[1;34m", // Debug:    Blue
		"\033[0m",    // Info:     Normal
		"\033[1;32m", // Warn:     Green
		"\033[1;33m", // Error:    Yellow
		"\033[1;31m"  // Critical: Red
	},

	.color_def = "\033[0m"

};

log_writer *log_writer_terminal_color = &log_terminal_color.writer;


static void file_plain_create( log_writer *self, const char *out);
static void file_plain_destroy(log_writer *self);
static void file_plain_write(  log_writer *self, int level, const char *buf, int len);

static struct _log_file_plain_t {

	log_writer  writer;
	FILE       *file;

} log_file_plain = {

	.writer = {
		.create  = (log_create_t)&file_plain_create,
		.destroy = (log_destroy_t)&file_plain_destroy,
		.write   = (log_write_t)&file_plain_write
	},

	.file = NULL

};

log_writer *log_writer_file_plain = &log_file_plain.writer;


static void file_html_create( log_writer *self, const char *out);
static void file_html_destroy(log_writer *self);
static void file_html_write(  log_writer *self, int level, const char *buf, int len);

static struct _log_file_html_t {

	log_writer writer;

	const char *style[5];

} log_file_html = {

	.writer = {
		.create  = (log_create_t)&file_html_create,
		.destroy = (log_destroy_t)&file_html_destroy,
		.write   = (log_write_t)&file_html_write
	},

	.style = {
		"", // Debug:    Blue
		"", // Info:     Normal
		"", // Warn:     Green
		"", // Error:    Yellow
		""  // Critical: Red
	}

};

log_writer *log_writer_file_html = &log_file_html.writer;



void log_create()
{
}



void log_destroy()
{

	for (int n=0; n < LOG_MAX_SLOTS; n++)
		log_set_writer(n, NULL, NULL);

}



void log_printf(int level, const char *msg, ...)
{

	va_list argv;
	int     size = 1024;

	if (level > LOG_CRITICAL)
		level = LOG_CRITICAL;

	while (true) {

		va_start(argv, msg);

		char buf[size];
		int len = vsnprintf(buf, size, msg, argv);

		va_end(argv);

		if (len < 0 || len >= size) {

			size *= 2;
			continue;

		}

		for (int n=0; n < LOG_MAX_SLOTS; n++)
			if (log_writers[n] != NULL && log_levels[n] <= level)
				(log_writers[n]->write)(log_writers[n], level, buf, len);

		break;

	}

}



void log_hexdump(int level, const void *buf, size_t len)
{

	static const int stride = 16;
	char cv[stride + 1];
	char hv[3 * stride + 1];

	const void *addr = buf;
	const u8   *ptr  = buf;

	memset(cv, ' ', sizeof(cv));
	memset(hv, ' ', sizeof(hv));

	cv[stride] = 0;
	hv[3 * stride] = 0;

	for (int n=0, m=0; n < len; n++) {

		cv[m] = isprint(*ptr)? *ptr: '.';
		sprintf(&hv[3 * m], " %02x", *ptr);

		ptr++;
		m++;

		if (n > 0 && (m == stride || n == len - 1)) {

			log_printf(level, "%p |%s | %s", addr, hv, cv);

			memset(cv, ' ', sizeof(cv));
			memset(hv, ' ', sizeof(hv));

			cv[stride] = 0;
			hv[3 * stride] = 0;

			addr += stride;
			m     = 0;

		}

	}

}



int log_get_slot()
{

	for (int n=0; n < LOG_MAX_SLOTS; n++)
		if (log_writers[n] == NULL)
			return n;

	return -1;

}



const log_writer *log_get_writer(uint slot)
{

	return (slot >= LOG_MAX_SLOTS)? NULL: log_writers[slot];

}



void log_set_writer(uint slot, log_writer *writer, const void *arg)
{

	if (slot >= LOG_MAX_SLOTS)
		return;

	if (log_writers[slot] != NULL)
		(log_writers[slot]->destroy)(log_writers[slot]);

	log_writers[slot] = writer;

	if (log_writers[slot] != NULL)
		(log_writers[slot]->create)(log_writers[slot], arg);

}



int log_get_level(uint slot)
{

	return (slot >= LOG_MAX_SLOTS)? -1: log_levels[slot];

}



void log_set_level(uint slot, int level)
{

	if (level > LOG_CRITICAL)
		level = LOG_CRITICAL;

	if (slot < LOG_MAX_SLOTS)
		log_levels[slot] = level;

}



void terminal_plain_create(log_writer *self, const void *arg)
{
}



void terminal_plain_destroy(log_writer *self)
{
}



void terminal_plain_write(log_writer *self, int level, const char *buf, int len)
{
	const char *level_text[5]={
		"",
		"",
		"warning: ",
		"error: ",
		"critical: "
	};

	fputs(level_text[level], stdout);
	fputs(buf,  stdout);
	fputc('\n', stdout);

}



void terminal_color_create(log_writer *self, const void *arg)
{
}



void terminal_color_destroy(log_writer *self)
{
}



void terminal_color_write(log_writer *self, int level, const char *buf, int len)
{
	struct _log_terminal_color_t *ltc = (struct _log_terminal_color_t*)self;
	const char *level_text[5]={
		"",
		"",
		"warning: ",
		"error: ",
		"critical: "
	};

	fputs(ltc->colors[level], stdout);
	fputs(level_text[level], stdout);
	fputs(buf, stdout);
	fputs(ltc->color_def, stdout);
	fputc('\n', stdout);

}



void file_plain_create(log_writer *self, const char *out)
{
	struct _log_file_plain_t *lfp = (struct _log_file_plain_t*)self;

	if (lfp->file != NULL)
		fclose(lfp->file);

	lfp->file = fopen(out, "w");

}



void file_plain_destroy(log_writer *self)
{
	struct _log_file_plain_t *lfp = (struct _log_file_plain_t*)self;

	if (lfp->file != NULL)
		fclose(lfp->file);

	lfp->file = NULL;

}



void file_plain_write(log_writer *self, int level, const char *buf, int len)
{
	struct _log_file_plain_t *lfp = (struct _log_file_plain_t*)self;
	const char *level_text[5]={
		"",
		"",
		"warning: ",
		"error: ",
		"critical: "
	};

	fputs(level_text[level], lfp->file);
	fputs(buf,  lfp->file);
	fputc('\n', lfp->file);

}



void file_html_create(log_writer *self, const char *out)
{
	struct _log_file_html_t *lfh = (struct _log_file_html_t*)self;
}



void file_html_destroy(log_writer *self)
{
	struct _log_file_html_t *lfh = (struct _log_file_html_t*)self;
}



void file_html_write(log_writer *self, int level, const char *buf, int len)
{
	struct _log_file_html_t *lfh = (struct _log_file_html_t*)self;
}

