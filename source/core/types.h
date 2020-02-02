

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

typedef unsigned int    uint;
typedef unsigned short  ushort;
typedef unsigned char   byte;


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


#include "druntime.h"
#include "gpuio.h"


#endif

