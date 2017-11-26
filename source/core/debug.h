

#ifndef CORE_DEBUG_H
#define CORE_DEBUG_H


#undef checkpoint
#undef assert
#undef value


#if 1//ef DEBUG

#define checkpoint() \
	do { \
		log_printf(LOG_DEBUG, "%s, %d: check!", __FILE__, __LINE__); \
	} while (0)

#define assert(pred) \
	do { \
		if (!(pred)) \
			log_printf(LOG_DEBUG, "%s, %d: assertion '%s' failed!", __FILE__, __LINE__, #pred); \
	} while (0)

#define watch(format, var...) \
	do { \
		log_printf(LOG_DEBUG, "%s, %d: %s = " format, __FILE__, __LINE__, #var, var); \
	} while(0)

#define log_d(format, ...) do { log_printf(LOG_DEBUG,    format, ## __VA_ARGS__); } while (0)
#define log_i(format, ...) do { log_printf(LOG_INFO,     format, ## __VA_ARGS__); } while (0)
#define log_w(format, ...) do { log_printf(LOG_WARN,     format, ## __VA_ARGS__); } while (0)
#define log_e(format, ...) do { log_printf(LOG_ERROR,    format, ## __VA_ARGS__); } while (0)
#define log_c(format, ...) do { log_printf(LOG_CRITICAL, format, ## __VA_ARGS__); } while (0)

#define autoconst volatile __attribute__((used,visibility("default")))

#else

#define checkpoint() { }    do { /* Nothing */ } while (0)
#define assert(pred) { }    do { /* Nothing */ } while (0)
#define watch(format, ...)  do { /* Nothing */ } while (0)
#define log(format, ...)    do { /* Nothing */ } while (0)

#define autoconst const

#endif


#endif

