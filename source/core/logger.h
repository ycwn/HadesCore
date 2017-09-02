

#ifndef __CORE_LOGGER_H
#define __CORE_LOGGER_H


#define LOG_MAX_SLOTS 4

enum {
	LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_CRITICAL
};

struct _log_writer_t;

typedef void (*log_create_t)( struct _log_writer_t *self, const void *arg);
typedef void (*log_destroy_t)(struct _log_writer_t *self);
typedef void (*log_write_t)(  struct _log_writer_t *self, int level, const char *buf, int len);

typedef struct _log_writer_t {

	log_create_t  create;
	log_destroy_t destroy;
	log_write_t   write;

} log_writer;


void log_create();
void log_destroy();

void log_printf(int level, const char *msg, ...);

int log_get_slot();

const log_writer *log_get_writer(uint slot);
void              log_set_writer(uint slot, log_writer *writer, const void *arg);

int  log_get_level(uint slot);
void log_set_level(uint slot, int level);


extern log_writer *log_writer_terminal_plain;
extern log_writer *log_writer_terminal_color;
extern log_writer *log_writer_file_plain;
extern log_writer *log_writer_file_html;


#endif

