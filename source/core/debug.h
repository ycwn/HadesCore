

#ifndef __CORE_DEBUG_H
#define __CORE_DEBUG_H


#undef checkpoint
#undef assert
#undef value


#if 1//ef DEBUG

#define checkpoint() \
	do { \
		fprintf(stderr, "%s, %d: check!\n", __FILE__, __LINE__); \
	} while (0)

#define assert(pred) \
	do { \
		if (!(pred)) \
			fprintf(stderr, "%s, %d: assertion '%s' failed!\n", __FILE__, __LINE__, #pred); \
	} while (0)

#define watch(format, var...) \
	do { \
		fprintf(stderr, "%s, %d: %s = " format "\n", __FILE__, __LINE__, #var, var); \
	} while(0)

#define log(format, ...) \
	do { \
		fprintf(stderr, format, ## __VA_ARGS__); \
	} while (0)

#define autoconst volatile __attribute__((used,visibility("default")))

#else

#define checkpoint() { }    do { /* Nothing */ } while (0)
#define assert(pred) { }    do { /* Nothing */ } while (0)
#define watch(format, ...)  do { /* Nothing */ } while (0)
#define log(format, ...)    do { /* Nothing */ } while (0)

#define autoconst const

#endif


#endif

