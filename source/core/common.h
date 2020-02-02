

#ifndef CORE_COMMON_H
#define CORE_COMMON_H


uint  pack_fp16(float x);
float unpack_fp16(uint x);

vec4 pack_m4_to_q4(const mat4 *m, int *handedness);
void unpack_m4_from_q4(mat4 *m, const vec4 q, int handedness);

vec4 calculate_tangent_q4 (const vec4 normal, const vec4 up);
vec4 calculate_tangent_q4h(const vec4 normal, const vec4 up);


static inline int   mini(int   x, int   y) { return (x < y)? x: y; }
static inline uint  minu(uint  x, uint  y) { return (x < y)? x: y; }
static inline float minf(float x, float y) { return (x < y)? x: y; }

static inline int   maxi(int   x, int   y) { return (x > y)? x: y; }
static inline uint  maxu(uint  x, uint  y) { return (x > y)? x: y; }
static inline float maxf(float x, float y) { return (x > y)? x: y; }

static inline int   clampi(int   x, int   min, int   max) { return mini(maxi(x, min), max); }
static inline uint  clampu(uint  x, uint  min, uint  max) { return minu(maxu(x, min), max); }
static inline float clampf(float x, float min, float max) { return minf(maxf(x, min), max); }


static inline uint pack_unorm16( float v)                   { return clampu(65535.0f * v,                                 0, 65535); }
static inline int  pack_snorm16( float v)                   { return clampi(32767.5f * v - 0.5f,                     -32768, 32767); }
static inline uint pack_uint16(  float v)                   { return clampu(v,                                            0, 65535); }
static inline int  pack_sint16(  float v)                   { return clampi(v,                                       -32768, 32767); }
static inline uint pack_urange16(float v, float a, float b) { return clampu(65535.0f * (v - a) / (b - a),                 0, 65535); }
static inline int  pack_srange16(float v, float a, float b) { return clampu(65535.0f * (v - a) / (b - a) - 32768.0f, -32768, 32767); }

static inline uint pack_r16_unorm( float r) { return pack_unorm16(r); }
static inline uint pack_r16_snorm( float r) { return pack_snorm16(r); }
static inline uint pack_r16_sfloat(float r) { return pack_fp16(r); }

static inline uint pack_r16g16_unorm( float r, float g) { return pack_unorm16(r) | (pack_unorm16(g) << 16); }
static inline uint pack_r16g16_snorm( float r, float g) { return pack_snorm16(r) | (pack_snorm16(g) << 16); }
static inline uint pack_r16g16_sfloat(float r, float g) { return pack_fp16(r) | (pack_fp16(g) << 16); }


static inline u32 pack_r11g11b10f(float r, float g, float b) {
	const u32 rr = (pack_fp16(r) >> 4)  & 0x000007ff;
	const u32 gg = (pack_fp16(g) << 7)  & 0x003ff800;
	const u32 bb = (pack_fp16(b) << 17) & 0xffc00000;
	return rr | gg | bb;
}

static inline u32 pack_r10g10b10a2_unorm(float r, float g, float b, float a) {
	const u32 rr = (clampu(r * 1023.0f, 0, 1023) <<  0) & 0x000003ff;
	const u32 gg = (clampu(g * 1023.0f, 0, 1023) << 10) & 0x000ffc00;
	const u32 bb = (clampu(b * 1023.0f, 0, 1023) << 20) & 0x3ff00000;
	const u32 aa = (clampu(a * 3.0f,    0, 3)    << 30) & 0xc0000000;
	return rr | gg | bb | aa;
}


static inline u32 pack_r10g10b10a2_snorm(float r, float g, float b, float a) {
	const u32 rr = (clampi(r * 511.5f - 0.5f, -512, 511) <<  0) & 0x000003ff;
	const u32 gg = (clampi(g * 511.5f - 0.5f, -512, 511) << 10) & 0x000ffc00;
	const u32 bb = (clampi(b * 511.5f - 0.5f, -512, 511) << 20) & 0x3ff00000;
	const u32 aa = (clampi(a * 1.5f   - 0.5f, -2,   1)   << 30) & 0xc0000000;
	return rr | gg | bb | aa;
}

static inline u32 pack_r10g10b10a2_uint(uint r, uint g, uint b, uint a) {
	const u32 rr = (clampu(r, 0, 1023) <<  0) & 0x000003ff;
	const u32 gg = (clampu(g, 0, 1023) << 10) & 0x000ffc00;
	const u32 bb = (clampu(b, 0, 1023) << 20) & 0x3ff00000;
	const u32 aa = (clampu(a, 0, 3)    << 30) & 0xc0000000;
	return rr | gg | bb | aa;
}

static inline u32 pack_r10g10b10a2_sint(int r, int g, int b, int a) {
	const u32 rr = (clampi(r, -512, 511) <<  0) & 0x000003ff;
	const u32 gg = (clampi(g, -512, 511) << 10) & 0x000ffc00;
	const u32 bb = (clampi(b, -512, 511) << 20) & 0x3ff00000;
	const u32 aa = (clampi(a, -2,   1)   << 30) & 0xc0000000;
	return rr | gg | bb | aa;
}


#define countof(x)    (sizeof(x) / sizeof((x)[0]))
#define mzero(x)      memset((x),  0, sizeof(x))
#define szero(x)      memset(&(x), 0, sizeof(x))
#define mfill(x, v)   do { for (int n=0; n < countof((x)); n++) (x)[n] = (v); } while (0)
#define bitmask(t, n) (((t) (-((n) != 0)))  & (((t) -1) >> ((sizeof(t) * 8) - (n))))


#endif

