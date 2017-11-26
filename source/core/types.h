

#ifndef CORE_TYPES_H
#define CORE_TYPES_H


typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef float   f32;
typedef double  f64;

typedef unsigned int  uint;


typedef float V2[2];
typedef float V3[3];
typedef float V4[4];

typedef float M3[9];
typedef float M4[16];


typedef simd4f   vec4;
typedef simd4x4f mat4;


typedef struct graphics graphics;
typedef struct sound    sound;
typedef struct input    input;


static inline int   mini(int   x, int   y) { return (x < y)? x: y; }
static inline uint  minu(uint  x, uint  y) { return (x < y)? x: y; }
static inline float minf(float x, float y) { return (x < y)? x: y; }

static inline int   maxi(int   x, int   y) { return (x > y)? x: y; }
static inline uint  maxu(uint  x, uint  y) { return (x > y)? x: y; }
static inline float maxf(float x, float y) { return (x > y)? x: y; }

static inline int   clampi(int   x, int   min, int   max) { return mini(maxi(x, min), max); }
static inline uint  clampu(uint  x, uint  min, uint  max) { return minu(maxu(x, min), max); }
static inline float clampf(float x, float min, float max) { return minf(maxf(x, min), max); }


#define countof(x) (sizeof(x) / sizeof((x)[0]))
#define mzero(x)   memset((x),  0, sizeof(x))
#define szero(x)   memset(&(x), 0, sizeof(x))


#include "druntime.h"
#include "gpuio.h"


#endif

