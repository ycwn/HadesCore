

#include "core/system.h"
#include "core/types.h"
#include "core/common.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/pixelformat.h"


#define LINEAR  GR_PIXELFORMAT_PACKING_LINEAR
#define BLOCK   GR_PIXELFORMAT_PACKING_BLOCK
#define PACKED  GR_PIXELFORMAT_PACKING_PACKED

#define C   VK_IMAGE_ASPECT_COLOR_BIT
#define D   VK_IMAGE_ASPECT_DEPTH_BIT
#define S   VK_IMAGE_ASPECT_STENCIL_BIT
#define DS  VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT


static gr_pixelformat formats[]={
	{ "undefined",            -1, 0, 0,    0,  0,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  0, { 0 } },
	{ "r4g4-unorm",           -1, 2, C,    8,  4,  4,  0,  0,  0, 0, PACKED,  0,  0, 0,  1, { 0 } },
	{ "r4g4b4a4-unorm",       -1, 4, C,   16,  4,  4,  4,  4,  0, 0, PACKED,  0,  0, 0,  2, { 0 } },
	{ "b4g4r4a4-unorm",       -1, 4, C,   16,  4,  4,  4,  4,  0, 0, PACKED,  0,  0, 0,  2, { 0 } },
	{ "r5g6b5-unorm",         -1, 3, C,   16,  5,  6,  5,  0,  0, 0, PACKED,  0,  0, 0,  2, { 0 } },
	{ "b5g6r5-unorm",         -1, 3, C,   16,  5,  6,  5,  0,  0, 0, PACKED,  0,  0, 0,  2, { 0 } },
	{ "r5g5b5a1-unorm",       -1, 4, C,   16,  5,  5,  5,  1,  0, 0, PACKED,  0,  0, 0,  2, { 0 } },
	{ "b5g5r5a1-unorm",       -1, 4, C,   16,  5,  5,  5,  1,  0, 0, PACKED,  0,  0, 0,  2, { 0 } },
	{ "a1r5g5b5-unorm",       -1, 4, C,   16,  5,  5,  5,  1,  0, 0, PACKED,  0,  0, 0,  2, { 0 } },
	{ "r8-unorm",             -1, 1, C,    8,  8,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  1, { 0 } },
	{ "r8-snorm",             -1, 1, C,    8,  8,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  1, { 0 } },
	{ "r8-uscaled",           -1, 1, C,    8,  8,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  1, { 0 } },
	{ "r8-sscaled",           -1, 1, C,    8,  8,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  1, { 0 } },
	{ "r8-uint",              -1, 1, C,    8,  8,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  1, { 0 } },
	{ "r8-sint",              -1, 1, C,    8,  8,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  1, { 0 } },
	{ "r8-srgb",              -1, 1, C,    8,  8,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  1, { 0 } },
	{ "r8g8-unorm",           -1, 2, C,   16,  8,  8,  0,  0,  0, 0, LINEAR,  0,  0, 0,  2, { 0 } },
	{ "r8g8-snorm",           -1, 2, C,   16,  8,  8,  0,  0,  0, 0, LINEAR,  0,  0, 0,  2, { 0 } },
	{ "r8g8-uscaled",         -1, 2, C,   16,  8,  8,  0,  0,  0, 0, LINEAR,  0,  0, 0,  2, { 0 } },
	{ "r8g8-sscaled",         -1, 2, C,   16,  8,  8,  0,  0,  0, 0, LINEAR,  0,  0, 0,  2, { 0 } },
	{ "r8g8-uint",            -1, 2, C,   16,  8,  8,  0,  0,  0, 0, LINEAR,  0,  0, 0,  2, { 0 } },
	{ "r8g8-sint",            -1, 2, C,   16,  8,  8,  0,  0,  0, 0, LINEAR,  0,  0, 0,  2, { 0 } },
	{ "r8g8-srgb",            -1, 2, C,   16,  8,  8,  0,  0,  0, 0, LINEAR,  0,  0, 0,  2, { 0 } },
	{ "r8g8b8-unorm",         -1, 3, C,   24,  8,  8,  8,  0,  0, 0, LINEAR,  0,  0, 0,  3, { 0 } },
	{ "r8g8b8-snorm",         -1, 3, C,   24,  8,  8,  8,  0,  0, 0, LINEAR,  0,  0, 0,  3, { 0 } },
	{ "r8g8b8-uscaled",       -1, 3, C,   24,  8,  8,  8,  0,  0, 0, LINEAR,  0,  0, 0,  3, { 0 } },
	{ "r8g8b8-sscaled",       -1, 3, C,   24,  8,  8,  8,  0,  0, 0, LINEAR,  0,  0, 0,  3, { 0 } },
	{ "r8g8b8-uint",          -1, 3, C,   24,  8,  8,  8,  0,  0, 0, LINEAR,  0,  0, 0,  3, { 0 } },
	{ "r8g8b8-sint",          -1, 3, C,   24,  8,  8,  8,  0,  0, 0, LINEAR,  0,  0, 0,  3, { 0 } },
	{ "r8g8b8-srgb",          -1, 3, C,   24,  8,  8,  8,  0,  0, 0, LINEAR,  0,  0, 0,  3, { 0 } },
	{ "b8g8r8-unorm",         -1, 3, C,   24,  8,  8,  8,  0,  0, 0, LINEAR,  0,  0, 0,  3, { 0 } },
	{ "b8g8r8-snorm",         -1, 3, C,   24,  8,  8,  8,  0,  0, 0, LINEAR,  0,  0, 0,  3, { 0 } },
	{ "b8g8r8-uscaled",       -1, 3, C,   24,  8,  8,  8,  0,  0, 0, LINEAR,  0,  0, 0,  3, { 0 } },
	{ "b8g8r8-sscaled",       -1, 3, C,   24,  8,  8,  8,  0,  0, 0, LINEAR,  0,  0, 0,  3, { 0 } },
	{ "b8g8r8-uint",          -1, 3, C,   24,  8,  8,  8,  0,  0, 0, LINEAR,  0,  0, 0,  3, { 0 } },
	{ "b8g8r8-sint",          -1, 3, C,   24,  8,  8,  8,  0,  0, 0, LINEAR,  0,  0, 0,  3, { 0 } },
	{ "b8g8r8-srgb",          -1, 3, C,   24,  8,  8,  8,  0,  0, 0, LINEAR,  0,  0, 0,  3, { 0 } },
	{ "r8g8b8a8-unorm",       -1, 4, C,   32,  8,  8,  8,  8,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r8g8b8a8-snorm",       -1, 4, C,   32,  8,  8,  8,  8,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r8g8b8a8-uscaled",     -1, 4, C,   32,  8,  8,  8,  8,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r8g8b8a8-sscaled",     -1, 4, C,   32,  8,  8,  8,  8,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r8g8b8a8-uint",        -1, 4, C,   32,  8,  8,  8,  8,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r8g8b8a8-sint",        -1, 4, C,   32,  8,  8,  8,  8,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r8g8b8a8-srgb",        -1, 4, C,   32,  8,  8,  8,  8,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "b8g8r8a8-unorm",       -1, 4, C,   32,  8,  8,  8,  8,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "b8g8r8a8-snorm",       -1, 4, C,   32,  8,  8,  8,  8,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "b8g8r8a8-uscaled",     -1, 4, C,   32,  8,  8,  8,  8,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "b8g8r8a8-sscaled",     -1, 4, C,   32,  8,  8,  8,  8,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "b8g8r8a8-uint",        -1, 4, C,   32,  8,  8,  8,  8,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "b8g8r8a8-sint",        -1, 4, C,   32,  8,  8,  8,  8,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "b8g8r8a8-srgb",        -1, 4, C,   32,  8,  8,  8,  8,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "a8b8g8r8-unorm",       -1, 4, C,   32,  8,  8,  8,  8,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a8b8g8r8-snorm",       -1, 4, C,   32,  8,  8,  8,  8,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a8b8g8r8-uscaled",     -1, 4, C,   32,  8,  8,  8,  8,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a8b8g8r8-sscaled",     -1, 4, C,   32,  8,  8,  8,  8,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a8b8g8r8-uint",        -1, 4, C,   32,  8,  8,  8,  8,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a8b8g8r8-sint",        -1, 4, C,   32,  8,  8,  8,  8,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a8b8g8r8-srgb",        -1, 4, C,   32,  8,  8,  8,  8,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a2r10g10b10-unorm",    -1, 4, C,   32, 10, 10, 10,  2,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a2r10g10b10-snorm",    -1, 4, C,   32, 10, 10, 10,  2,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a2r10g10b10-uscaled",  -1, 4, C,   32, 10, 10, 10,  2,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a2r10g10b10-sscaled",  -1, 4, C,   32, 10, 10, 10,  2,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a2r10g10b10-uint",     -1, 4, C,   32, 10, 10, 10,  2,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a2r10g10b10-sint",     -1, 4, C,   32, 10, 10, 10,  2,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a2b10g10r10-unorm",    -1, 4, C,   32, 10, 10, 10,  2,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a2b10g10r10-snorm",    -1, 4, C,   32, 10, 10, 10,  2,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a2b10g10r10-uscaled",  -1, 4, C,   32, 10, 10, 10,  2,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a2b10g10r10-sscaled",  -1, 4, C,   32, 10, 10, 10,  2,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a2b10g10r10-uint",     -1, 4, C,   32, 10, 10, 10,  2,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "a2b10g10r10-sint",     -1, 4, C,   32, 10, 10, 10,  2,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "r16-unorm",            -1, 1, C,   16, 16,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  2, { 0 } },
	{ "r16-snorm",            -1, 1, C,   16, 16,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  2, { 0 } },
	{ "r16-uscaled",          -1, 1, C,   16, 16,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  2, { 0 } },
	{ "r16-sscaled",          -1, 1, C,   16, 16,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  2, { 0 } },
	{ "r16-uint",             -1, 1, C,   16, 16,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  2, { 0 } },
	{ "r16-sint",             -1, 1, C,   16, 16,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  2, { 0 } },
	{ "r16-sfloat",           -1, 1, C,   16, 16,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  2, { 0 } },
	{ "r16g16-unorm",         -1, 2, C,   32, 16, 16,  0,  0,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r16g16-snorm",         -1, 2, C,   32, 16, 16,  0,  0,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r16g16-uscaled",       -1, 2, C,   32, 16, 16,  0,  0,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r16g16-sscaled",       -1, 2, C,   32, 16, 16,  0,  0,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r16g16-uint",          -1, 2, C,   32, 16, 16,  0,  0,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r16g16-sint",          -1, 2, C,   32, 16, 16,  0,  0,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r16g16-sfloat",        -1, 2, C,   32, 16, 16,  0,  0,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r16g16b16-unorm",      -1, 3, C,   48, 16, 16, 16,  0,  0, 0, LINEAR,  0,  0, 0,  6, { 0 } },
	{ "r16g16b16-snorm",      -1, 3, C,   48, 16, 16, 16,  0,  0, 0, LINEAR,  0,  0, 0,  6, { 0 } },
	{ "r16g16b16-uscaled",    -1, 3, C,   48, 16, 16, 16,  0,  0, 0, LINEAR,  0,  0, 0,  6, { 0 } },
	{ "r16g16b16-sscaled",    -1, 3, C,   48, 16, 16, 16,  0,  0, 0, LINEAR,  0,  0, 0,  6, { 0 } },
	{ "r16g16b16-uint",       -1, 3, C,   48, 16, 16, 16,  0,  0, 0, LINEAR,  0,  0, 0,  6, { 0 } },
	{ "r16g16b16-sint",       -1, 3, C,   48, 16, 16, 16,  0,  0, 0, LINEAR,  0,  0, 0,  6, { 0 } },
	{ "r16g16b16-sfloat",     -1, 3, C,   48, 16, 16, 16,  0,  0, 0, LINEAR,  0,  0, 0,  6, { 0 } },
	{ "r16g16b16a16-unorm",   -1, 4, C,   64, 16, 16, 16, 16,  0, 0, LINEAR,  0,  0, 0,  8, { 0 } },
	{ "r16g16b16a16-snorm",   -1, 4, C,   64, 16, 16, 16, 16,  0, 0, LINEAR,  0,  0, 0,  8, { 0 } },
	{ "r16g16b16a16-uscaled", -1, 4, C,   64, 16, 16, 16, 16,  0, 0, LINEAR,  0,  0, 0,  8, { 0 } },
	{ "r16g16b16a16-sscaled", -1, 4, C,   64, 16, 16, 16, 16,  0, 0, LINEAR,  0,  0, 0,  8, { 0 } },
	{ "r16g16b16a16-uint",    -1, 4, C,   64, 16, 16, 16, 16,  0, 0, LINEAR,  0,  0, 0,  8, { 0 } },
	{ "r16g16b16a16-sint",    -1, 4, C,   64, 16, 16, 16, 16,  0, 0, LINEAR,  0,  0, 0,  8, { 0 } },
	{ "r16g16b16a16-sfloat",  -1, 4, C,   64, 16, 16, 16, 16,  0, 0, LINEAR,  0,  0, 0,  8, { 0 } },
	{ "r32-uint",             -1, 1, C,   32, 32,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r32-sint",             -1, 1, C,   32, 32,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r32-sfloat",           -1, 1, C,   32, 32,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "r32g32-uint",          -1, 2, C,   64, 32, 32,  0,  0,  0, 0, LINEAR,  0,  0, 0,  8, { 0 } },
	{ "r32g32-sint",          -1, 2, C,   64, 32, 32,  0,  0,  0, 0, LINEAR,  0,  0, 0,  8, { 0 } },
	{ "r32g32-sfloat",        -1, 2, C,   64, 32, 32,  0,  0,  0, 0, LINEAR,  0,  0, 0,  8, { 0 } },
	{ "r32g32b32-uint",       -1, 3, C,   96, 32, 32, 32,  0,  0, 0, LINEAR,  0,  0, 0, 12, { 0 } },
	{ "r32g32b32-sint",       -1, 3, C,   96, 32, 32, 32,  0,  0, 0, LINEAR,  0,  0, 0, 12, { 0 } },
	{ "r32g32b32-sfloat",     -1, 3, C,   96, 32, 32, 32,  0,  0, 0, LINEAR,  0,  0, 0, 12, { 0 } },
	{ "r32g32b32a32-uint",    -1, 4, C,  128, 32, 32, 32, 32,  0, 0, LINEAR,  0,  0, 0, 16, { 0 } },
	{ "r32g32b32a32-sint",    -1, 4, C,  128, 32, 32, 32, 32,  0, 0, LINEAR,  0,  0, 0, 16, { 0 } },
	{ "r32g32b32a32-sfloat",  -1, 4, C,  128, 32, 32, 32, 32,  0, 0, LINEAR,  0,  0, 0, 16, { 0 } },
	{ "r64-uint",             -1, 1, C,   64, 64,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  8, { 0 } },
	{ "r64-sint",             -1, 1, C,   64, 64,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  8, { 0 } },
	{ "r64-sfloat",           -1, 1, C,   64, 64,  0,  0,  0,  0, 0, LINEAR,  0,  0, 0,  8, { 0 } },
	{ "r64g64-uint",          -1, 2, C,  128, 64, 64,  0,  0,  0, 0, LINEAR,  0,  0, 0, 16, { 0 } },
	{ "r64g64-sint",          -1, 2, C,  128, 64, 64,  0,  0,  0, 0, LINEAR,  0,  0, 0, 16, { 0 } },
	{ "r64g64-sfloat",        -1, 2, C,  128, 64, 64,  0,  0,  0, 0, LINEAR,  0,  0, 0, 16, { 0 } },
	{ "r64g64b64-uint",       -1, 3, C,  192, 64, 64, 64,  0,  0, 0, LINEAR,  0,  0, 0, 24, { 0 } },
	{ "r64g64b64-sint",       -1, 3, C,  192, 64, 64, 64,  0,  0, 0, LINEAR,  0,  0, 0, 24, { 0 } },
	{ "r64g64b64-sfloat",     -1, 3, C,  192, 64, 64, 64,  0,  0, 0, LINEAR,  0,  0, 0, 24, { 0 } },
	{ "r64g64b64a64-uint",    -1, 4, C,  256, 64, 64, 64, 64,  0, 0, LINEAR,  0,  0, 0, 32, { 0 } },
	{ "r64g64b64a64-sint",    -1, 4, C,  256, 64, 64, 64, 64,  0, 0, LINEAR,  0,  0, 0, 32, { 0 } },
	{ "r64g64b64a64-sfloat",  -1, 4, C,  256, 64, 64, 64, 64,  0, 0, LINEAR,  0,  0, 0, 32, { 0 } },
	{ "b10g11r11-ufloat",     -1, 3, C,   32, 11, 11, 10,  0,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "e5b9g9r9-ufloat",      -1, 3, C,   32, 14, 14, 14,  0,  0, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "d16-unorm",            -1, 1, D,   16,  0,  0,  0,  0, 16, 0, LINEAR,  0,  0, 0,  2, { 0 } },
	{ "x8-d24-unorm",         -1, 1, D,   32,  0,  0,  0,  0, 24, 0, PACKED,  0,  0, 0,  4, { 0 } },
	{ "d32-sfloat",           -1, 1, D,   32,  0,  0,  0,  0, 32, 0, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "s8-uint",              -1, 1, S,    8,  0,  0,  0,  0, 0,  8, LINEAR,  0,  0, 0,  1, { 0 } },
	{ "d16-unorm-s8-uint",    -1, 2, DS,  24,  0,  0,  0,  0, 16, 8, LINEAR,  0,  0, 0,  3, { 0 } },
	{ "d24-unorm-s8-uint",    -1, 2, DS,  32,  0,  0,  0,  0, 24, 8, LINEAR,  0,  0, 0,  4, { 0 } },
	{ "d32-sfloat-s8-uint",   -1, 2, DS,  40,  0,  0,  0,  0, 32, 8, LINEAR,  0,  0, 0,  5, { 0 } },
	{ "bc1-rgb-unorm",        -1, 3, C,    4,  5,  6,  5,  0,  0, 0, BLOCK,   4,  4, 1,  8, { 0 } },
	{ "bc1-rgb-srgb",         -1, 3, C,    4,  5,  6,  5,  0,  0, 0, BLOCK,   4,  4, 1,  8, { 0 } },
	{ "bc1-rgba-unorm",       -1, 4, C,    4,  5,  6,  5,  1,  0, 0, BLOCK,   4,  4, 1,  8, { 0 } },
	{ "bc1-rgba-srgb",        -1, 4, C,    4,  5,  6,  5,  1,  0, 0, BLOCK,   4,  4, 1,  8, { 0 } },
	{ "bc2-unorm",            -1, 4, C,    8,  5,  6,  5,  4,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "bc2-srgb",             -1, 4, C,    8,  5,  6,  5,  4,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "bc3-unorm",            -1, 4, C,    8,  5,  6,  5,  8,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "bc3-srgb",             -1, 4, C,    8,  5,  6,  5,  8,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "bc4-unorm",            -1, 1, C,    4,  8,  0,  0,  0,  0, 0, BLOCK,   4,  4, 1,  8, { 0 } },
	{ "bc4-snorm",            -1, 1, C,    4,  8,  0,  0,  0,  0, 0, BLOCK,   4,  4, 1,  8, { 0 } },
	{ "bc5-unorm",            -1, 2, C,    8,  8,  8,  0,  0,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "bc5-snorm",            -1, 2, C,    8,  8,  8,  0,  0,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "bc6h-ufloat",          -1, 3, C,    8, 16, 16, 16,  0,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "bc6h-sfloat",          -1, 3, C,    8, 16, 16, 16,  0,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "bc7-unorm",            -1, 4, C,    8,  7,  7,  7,  8,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "bc7-srgb",             -1, 4, C,    8,  7,  7,  7,  8,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "etc2-r8g8b8-unorm",    -1, 3, C,    4,  8,  8,  8,  0,  0, 0, BLOCK,   4,  4, 1,  8, { 0 } },
	{ "etc2-r8g8b8-srgb",     -1, 3, C,    4,  8,  8,  8,  0,  0, 0, BLOCK,   4,  4, 1,  8, { 0 } },
	{ "etc2-r8g8b8a1-unorm",  -1, 4, C,    4,  8,  8,  8,  1,  0, 0, BLOCK,   4,  4, 1,  8, { 0 } },
	{ "etc2-r8g8b8a1-srgb",   -1, 4, C,    4,  8,  8,  8,  1,  0, 0, BLOCK,   4,  4, 1,  8, { 0 } },
	{ "etc2-r8g8b8a8-unorm",  -1, 4, C,    8,  8,  8,  8,  8,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "etc2-r8g8b8a8-srgb",   -1, 4, C,    8,  8,  8,  8,  8,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "eac-r11-unorm",        -1, 1, C,    4, 11,  0,  0,  0,  0, 0, BLOCK,   4,  4, 1,  8, { 0 } },
	{ "eac-r11-snorm",        -1, 1, C,    4, 11,  0,  0,  0,  0, 0, BLOCK,   4,  4, 1,  8, { 0 } },
	{ "eac-r11g11-unorm",     -1, 2, C,    8, 11, 11,  0,  0,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "eac-r11g11-snorm",     -1, 2, C,    8, 11, 11,  0,  0,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "astc-4x4-unorm",       -1, 4, C,    8,  8,  8,  8,  8,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "astc-4x4-srgb",        -1, 4, C,    8,  8,  8,  8,  8,  0, 0, BLOCK,   4,  4, 1, 16, { 0 } },
	{ "astc-5x4-unorm",       -1, 4, C,    6,  8,  8,  8,  8,  0, 0, BLOCK,   5,  4, 1, 16, { 0 } },
	{ "astc-5x4-srgb",        -1, 4, C,    6,  8,  8,  8,  8,  0, 0, BLOCK,   5,  4, 1, 16, { 0 } },
	{ "astc-5x5-unorm",       -1, 4, C,    5,  8,  8,  8,  8,  0, 0, BLOCK,   5,  5, 1, 16, { 0 } },
	{ "astc-5x5-srgb",        -1, 4, C,    5,  8,  8,  8,  8,  0, 0, BLOCK,   5,  5, 1, 16, { 0 } },
	{ "astc-6x5-unorm",       -1, 4, C,    4,  8,  8,  8,  8,  0, 0, BLOCK,   6,  5, 1, 16, { 0 } },
	{ "astc-6x5-srgb",        -1, 4, C,    4,  8,  8,  8,  8,  0, 0, BLOCK,   6,  5, 1, 16, { 0 } },
	{ "astc-6x6-unorm",       -1, 4, C,    4,  8,  8,  8,  8,  0, 0, BLOCK,   6,  6, 1, 16, { 0 } },
	{ "astc-6x6-srgb",        -1, 4, C,    4,  8,  8,  8,  8,  0, 0, BLOCK,   6,  6, 1, 16, { 0 } },
	{ "astc-8x5-unorm",       -1, 4, C,    3,  8,  8,  8,  8,  0, 0, BLOCK,   8,  5, 1, 16, { 0 } },
	{ "astc-8x5-srgb",        -1, 4, C,    3,  8,  8,  8,  8,  0, 0, BLOCK,   8,  5, 1, 16, { 0 } },
	{ "astc-8x6-unorm",       -1, 4, C,    3,  8,  8,  8,  8,  0, 0, BLOCK,   8,  6, 1, 16, { 0 } },
	{ "astc-8x6-srgb",        -1, 4, C,    3,  8,  8,  8,  8,  0, 0, BLOCK,   8,  6, 1, 16, { 0 } },
	{ "astc-8x8-unorm",       -1, 4, C,    2,  8,  8,  8,  8,  0, 0, BLOCK,   8,  8, 1, 16, { 0 } },
	{ "astc-8x8-srgb",        -1, 4, C,    2,  8,  8,  8,  8,  0, 0, BLOCK,   8,  8, 1, 16, { 0 } },
	{ "astc-10x5-unorm",      -1, 4, C,    3,  8,  8,  8,  8,  0, 0, BLOCK,  10,  5, 1, 16, { 0 } },
	{ "astc-10x5-srgb",       -1, 4, C,    3,  8,  8,  8,  8,  0, 0, BLOCK,  10,  5, 1, 16, { 0 } },
	{ "astc-10x6-unorm",      -1, 4, C,    2,  8,  8,  8,  8,  0, 0, BLOCK,  10,  6, 1, 16, { 0 } },
	{ "astc-10x6-srgb",       -1, 4, C,    2,  8,  8,  8,  8,  0, 0, BLOCK,  10,  6, 1, 16, { 0 } },
	{ "astc-10x8-unorm",      -1, 4, C,    2,  8,  8,  8,  8,  0, 0, BLOCK,  10,  8, 1, 16, { 0 } },
	{ "astc-10x8-srgb",       -1, 4, C,    2,  8,  8,  8,  8,  0, 0, BLOCK,  10,  8, 1, 16, { 0 } },
	{ "astc-10x10-unorm",     -1, 4, C,    1,  8,  8,  8,  8,  0, 0, BLOCK,  10, 10, 1, 16, { 0 } },
	{ "astc-10x10-srgb",      -1, 4, C,    1,  8,  8,  8,  8,  0, 0, BLOCK,  10, 10, 1, 16, { 0 } },
	{ "astc-12x10-unorm",     -1, 4, C,    1,  8,  8,  8,  8,  0, 0, BLOCK,  12, 10, 1, 16, { 0 } },
	{ "astc-12x10-srgb",      -1, 4, C,    1,  8,  8,  8,  8,  0, 0, BLOCK,  12, 10, 1, 16, { 0 } },
	{ "astc-12x12-unorm",     -1, 4, C,    1,  8,  8,  8,  8,  0, 0, BLOCK,  12, 12, 1, 16, { 0 } },
	{ "astc-12x12-srgb",      -1, 4, C,    1,  8,  8,  8,  8,  0, 0, BLOCK,  12, 12, 1, 16, { 0 } }

};

static graphics *gfx = NULL;



void gr_pixelformat_create(graphics *gr)
{

	gfx = gr;

	for (int n=0; n < countof(formats); n++)
		formats[n].format = n;

}



void gr_pixelformat_destroy()
{
}



bool gr_pixelformat_init()
{

	for (int n=0; n < countof(formats); n++)
		vkGetPhysicalDeviceFormatProperties(gfx->vk.device, formats[n].format, &formats[n].properties);

	return true;

}



const gr_pixelformat *gr_pixelformat_get(int format)
{

	if (format < 0 || format >= countof(formats))
		return &formats[VK_FORMAT_UNDEFINED];

	return &formats[format];

}



const gr_pixelformat *gr_pixelformat_find(const char *name)
{

	for (int n=0; n < countof(formats); n++)
		if (!strcmp(formats[n].name, name))
			return &formats[n];

	return &formats[VK_FORMAT_UNDEFINED];

}

