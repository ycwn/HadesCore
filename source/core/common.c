

#include "core/system.h"
#include "core/types.h"
#include "core/common.h"


union float_bits
{
	float f;
	i32   si;
	u32 ui;
};

static const int shift = 13;
static const int shiftSign = 16;

static const i32 infN = 0x7F800000; // flt32 infinity
static const i32 maxN = 0x477FE000; // max flt16 normal as a flt32
static const i32 minN = 0x38800000; // min flt16 normal as a flt32
static const i32 signN = 0x80000000; // flt32 sign bit

static const i32 infC = infN >> shift;
static const i32 nanN = (infC + 1) << shift; // minimum flt16 nan as a flt32
static const i32 maxC = maxN >> shift;
static const i32 minC = minN >> shift;
static const i32 signC = signN >> shiftSign; // flt16 sign bit

static const i32 mulN = 0x52000000; // (1 << 23) / minN
static const i32 mulC = 0x33800000; // minN / (1 << (23 - shift))

static const i32 subC = 0x003FF; // max flt32 subnormal down shifted
static const i32 norC = 0x00400; // min flt32 normal down shifted

static const i32 maxD = infC - maxC - 1;
static const i32 minD = minC - subC - 1;



uint pack_fp16(float x)
{

	volatile union float_bits v, s;

	v.f = x;
	u32 sign = v.si & signN;
	v.si ^= sign;
	sign >>= shiftSign;
	s.si = mulN;

	s.si = s.f * v.f;
	v.si ^= (s.si ^ v.si) & -(minN > v.si);
	v.si ^= (infN ^ v.si) & -((infN > v.si) & (v.si > maxN));
	v.si ^= (nanN ^ v.si) & -((nanN > v.si) & (v.si > infN));

	v.ui >>= shift;
	v.si ^= ((v.si - maxD) ^ v.si) & -(v.si > maxC);
	v.si ^= ((v.si - minD) ^ v.si) & -(v.si > subC);

	return v.ui | sign;

}



float unpack_fp16(uint x)
{

	volatile union float_bits v, s;

	v.ui = x;
	i32 sign = v.si & signC;
	v.si ^= sign;

	sign <<= shiftSign;
	v.si ^= ((v.si + minD) ^ v.si) & -(v.si > subC);
	v.si ^= ((v.si + maxD) ^ v.si) & -(v.si > maxC);

	s.si = mulC;
	s.f *= v.si;

	i32 mask = -(norC > v.si);

	v.si <<= shift;
	v.si ^= (s.si ^ v.si) & mask;
	v.si |= sign;

	return v.f;

}



vec4 pack_m4_to_q4(const mat4 *m, int *handedness)
{

	mat4 tmp;
	vec4 q;

	float det = simd4f_get_x(simd4x4f_inverse(m, &tmp));

	if (handedness != NULL)
		*handedness = (det < 0.0f)? -1: +1;

	const float m00 = simd4f_get_x(m->x);
	const float m01 = simd4f_get_y(m->x);
	const float m02 = simd4f_get_z(m->x);
	const float m10 = simd4f_get_x(m->y);
	const float m11 = simd4f_get_y(m->y);
	const float m12 = simd4f_get_z(m->y);
	const float m20 = (det < 0.0f)? simd4f_get_x(m->z): +simd4f_get_x(m->z);
	const float m21 = (det < 0.0f)? simd4f_get_y(m->z): +simd4f_get_y(m->z);
	const float m22 = (det < 0.0f)? simd4f_get_z(m->z): +simd4f_get_z(m->z);

	const float trace = m00 + m11 + m22;

	if (trace > 0.0f) {

		const float sigma = sqrtf(trace + 1.0f) * 2.0f;

		q = simd4f_create(
			(m21 - m12) / sigma,
			(m02 - m20) / sigma,
			(m10 - m01) / sigma,
			sigma / 4.0f);

	} else if ((m00 > m11) && (m00 > m22)) {

		const float sigma = sqrtf(1.0f + m00 - m11 - m22) * 2.0f;

		q = simd4f_create(
			sigma / 4.0f,
			(m01 + m10) / sigma,
			(m02 + m20) / sigma,
			(m21 - m12) / sigma);

	} else if (m11 > m22) {

		const float sigma = sqrtf(1.0f + m11 - m00 - m22) * 2.0f;

		q = simd4f_create(
			(m01 + m10) / sigma,
			sigma / 4.0f,
			(m12 + m21) / sigma,
			(m02 - m20) / sigma);

	} else {

		const float sigma = sqrtf(1.0f + m22 - m00 - m11) * 2.0f;

		q = simd4f_create(
			(m02 + m20) / sigma,
			(m12 + m21) / sigma,
			sigma / 4.0f,
			(m10 - m01) / sigma);

	}

	if (simd4f_get_w(q) < 0.0f)
		q = simd4f_flip_sign_0101(simd4f_flip_sign_1010(q));

	return q;

}



void unpack_m4_from_q4(mat4 *m, const vec4 q, int handedness)
{

	const float qx = simd4f_get_x(q);
	const float qy = simd4f_get_y(q);
	const float qz = simd4f_get_z(q);
	const float qw = simd4f_get_w(q);

	m->x = simd4f_create(
		1.0f - 2.0f * (qy * qy + qz * qz),
		       2.0f * (qx * qy + qz * qw),
		       2.0f * (qx * qz - qy * qw),
		0.0f);

	m->y = simd4f_create(
		       2.0f * (qx * qy - qz * qw),
		1.0f - 2.0f * (qx * qx + qz * qz),
		       2.0f * (qy * qz + qx * qw),
		0.0f);

	m->z = simd4f_create(
		       2.0f * (qx * qz + qy * qw),
		       2.0f * (qy * qz - qx * qw),
		1.0f - 2.0f * (qx * qx + qy * qy),
		0.0f);

	m->w = simd4f_create(
		0.0f,
		0.0f,
		0.0f,
		1.0f);

	if (handedness < 0)
		m->z = simd4f_flip_sign_0101(simd4f_flip_sign_1010(m->z));

}



vec4 calculate_tangent_q4(const vec4 normal, const vec4 up)
{

	const vec4 N = simd4f_normalize3(normal);
	const vec4 X = simd4f_normalize3(up);

	const vec4 R = simd4f_normalize3(simd4f_cross3(N, X));
	const vec4 U = simd4f_normalize3(simd4f_cross3(N, R)); //TODO: Check order

	const mat4 tangent = simd4x4f_create(
		R, U, N, simd4f_create(0.0f, 0.0f, 0.0f, 1.0f)
	);

	return pack_m4_to_q4(&tangent, NULL);

}



vec4 calculate_tangent_q4h(const vec4 normal, const vec4 up)
{

	const vec4 N = simd4f_normalize3(normal);
	const vec4 X = simd4f_normalize3(up);

	const vec4 R = simd4f_normalize3(simd4f_cross3(N, X));
	const vec4 U = simd4f_normalize3(simd4f_cross3(N, R)); //TODO: Check order

	const mat4 tangent = simd4x4f_create(
		R, U, N, simd4f_create(0.0f, 0.0f, 0.0f, 1.0f)
	);

	int        h = 0;
	const vec4 q = simd4f_zero_w(pack_m4_to_q4(&tangent, &h));
	const vec4 p = simd4f_create(0.0f, 0.0f, 0.0f, 1.0f);

	return (h < 0)? q - p: q + p;

}
