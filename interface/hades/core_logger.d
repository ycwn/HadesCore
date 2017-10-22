

enum LOG_MAX_SLOTS = 4;

enum {
	LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_CRITICAL
}


alias log_writer_create  = void function(log_writer *self, const void *arg);
alias log_writer_destroy = void function(log_writer *self);
alias log_writer_write   = void function(log_writer *self, int level, const char *buf, int len);

struct log_writer {

	log_writer_create  create;
	log_writer_destroy destroy;
	log_writer_write   write;

}


extern(C) {

	void log_create();
	void log_destroy();

	void log_printf(int level, const char *msg, ...);

	int log_get_slot();

	const(log_writer) *log_get_writer(uint slot);
	void               log_set_writer(uint slot, log_writer *writer, const void *arg);

	int  log_get_level(uint slot);
	void log_set_level(uint slot, int level);


	extern log_writer *log_writer_terminal_plain;
	extern log_writer *log_writer_terminal_color;
	extern log_writer *log_writer_file_plain;
	extern log_writer *log_writer_file_html;

}

