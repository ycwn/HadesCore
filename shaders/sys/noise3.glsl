
/**
 *
 *  3D GLSL noise functions
 *
 * Copyright (c) 2016 Stefanos Chrysafis. All rights reserved.
 * Distributed under the MIT license. See LICENSE.
 * https://github.com/ycwn/glsl-noise
 *
 */


/**
 *   The constants below were picked (almost) randomly from a list
 *  of prime numbers. The operations done ensure that the numbers
 *  don't factor exactly, and that we have enough fractional digits.
 */
const float A0 = 1.8005074564545359390483674186325e+2;  // 102497 * sqrt(pi) / 1009
const float A1 = 1.8002263934058294151986471938581e+2;  // 102481 * sqrt(pi) / 1009
const float A2 = 1.8006128550978008854920125029229e+2;  // 102503 * sqrt(pi) / 1009
const float A3 = 1.8005425893356242545295824467293e+2;  // 102499 * sqrt(pi) / 1009
const float B0 = 5.6718603649948929256653142163340e+1;  // sqrt(3217)
const float B1 = 5.6753854494650845696933117980929e+1;  // sqrt(3221)
const float B2 = 5.6824290580701489299609201674283e+1;  // sqrt(3229)
const float B3 = 5.7017541160593728289770111569626e+1;  // sqrt(3251)
const float C0 = 4.3762086387648384148423937870740e+4;  // 19571 * sqrt(5)
const float C1 = 4.3779246944185780501030235875469e+4;  // 16547 * sqrt(7)
const float C2 = 4.3736331110416657810278617998476e+4;  // 13187 * sqrt(11)
const float C3 = 4.3782209137959221986346703850894e+4;  // 12143 * sqrt(13)
const float M  = 1.1314698645845301288149239145544e+2;  // 2909 / sqrt(661)

float hash(float u, float v) { return fract(sin(mod(u, M)) * v); }


/**
 * Hash a vec4 and return a random float value
 *
 *   The idea behind this is to run the common one-liner random
 * generator for each component of the vector in a daisy-chain,
 * submitting each result to the next stage. This ensures that
 * the component hashing is done after the non-linear part. In
 * constrast, in the original one-liner this is done by a dot()
 * before the non-linear part.
 *
 */
float rand(vec4 u)
{
	return hash((u.x + hash((u.y + hash((u.z + hash(u.w * A3 + B3, C3)) * A2 + B2, C2)) * A1 + B1, C1)) * A0 + B0, C0);
}


/**
 * Hash a vec4 and return a ranndom float in the [-1.0, +1.0] range
 *
 */
float rand1(vec4 u)
{
	return 2.0 * rand(u) - 1.0;
}


/**
 * Hash a vec4 and return a uniform 2D vector on the unit circle
 *
 */
vec2 rand2(vec4 u)
{
	float t = 2.0 * rand(u) * 3.141592653589793;
	return vec2(cos(t), sin(t));
}


/**
 * Hash a vec4 and return a uniform 3D vector on the unit sphere
 *
 */
vec3 rand3(vec4 u)
{
	float z = 2.0 * rand(+u) - 1.0;
	float t = 2.0 * rand(-u) * 3.141592653589793;
	float r = sqrt(1.0 - z*z);
	return vec3(r * cos(t), r * sin(t), z);
}


float curve(float t) { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }
vec2  curve(vec2  t) { return t * t * t * (t * (t * 6.0 - vec2(15.0)) + vec2(10.0)); }
vec3  curve(vec3  t) { return t * t * t * (t * (t * 6.0 - vec3(15.0)) + vec3(10.0)); }
vec4  curve(vec4  t) { return t * t * t * (t * (t * 6.0 - vec4(15.0)) + vec4(10.0)); }


/**
 *  Calculate one sample of 4D Solid Noise.
 *  Return a uniform 3D vector on the unit sphere.
 *
 */
vec3 noise3(vec4 x)
{
	vec4 u = fract(x);
	vec4 v = x - u;
	vec4 c = curve(u);

	return  mix(mix(mix(mix(rand3(v + vec4(0.0, 0.0, 0.0, 0.0)), rand3(v + vec4(0.0, 0.0, 0.0, 1.0)), c.q),
						mix(rand3(v + vec4(0.0, 0.0, 1.0, 0.0)), rand3(v + vec4(0.0, 0.0, 1.0, 1.0)), c.q), c.p),
					mix(mix(rand3(v + vec4(0.0, 1.0, 0.0, 0.0)), rand3(v + vec4(0.0, 1.0, 0.0, 1.0)), c.q),
						mix(rand3(v + vec4(0.0, 1.0, 1.0, 0.0)), rand3(v + vec4(0.0, 1.0, 1.0, 1.0)), c.q), c.p), c.t),
				mix(mix(mix(rand3(v + vec4(1.0, 0.0, 0.0, 0.0)), rand3(v + vec4(1.0, 0.0, 0.0, 1.0)), c.q),
						mix(rand3(v + vec4(1.0, 0.0, 1.0, 0.0)), rand3(v + vec4(1.0, 0.0, 1.0, 1.0)), c.q), c.p),
					mix(mix(rand3(v + vec4(1.0, 1.0, 0.0, 0.0)), rand3(v + vec4(1.0, 1.0, 0.0, 1.0)), c.q),
						mix(rand3(v + vec4(1.0, 1.0, 1.0, 0.0)), rand3(v + vec4(1.0, 1.0, 1.0, 1.0)), c.q), c.p), c.t), c.s);

}


/**
 *  Calculate one sample of 3D Solid Noise.
 *  Return a uniform 3D vector on the unit sphere.
 *
 */
vec3 noise3(vec3 x)
{
	vec3 u = fract(x);
	vec4 v = vec4(x - u, 0.0);
	vec3 c = curve(u);

	return  mix(mix(mix(rand3(v + vec4(0.0, 0.0, 0.0, 0.0)), rand3(v + vec4(0.0, 0.0, 1.0, 0.0)), c.p),
					mix(rand3(v + vec4(0.0, 1.0, 0.0, 0.0)), rand3(v + vec4(0.0, 1.0, 1.0, 0.0)), c.p), c.t),
				mix(mix(rand3(v + vec4(1.0, 0.0, 0.0, 0.0)), rand3(v + vec4(1.0, 0.0, 1.0, 0.0)), c.p),
					mix(rand3(v + vec4(1.0, 1.0, 0.0, 0.0)), rand3(v + vec4(1.0, 1.0, 1.0, 0.0)), c.p), c.t), c.s);

}


/**
 *  Calculate one sample of 2D Solid Noise.
 *  Return a uniform 3D vector on the unit sphere.
 *
 */
vec3 noise3(vec2 x)
{
	vec2 u = fract(x);
	vec4 v = vec4(x - u, 0.0, 0.0);
	vec2 c = curve(u);

	return  mix(mix(rand3(v + vec4(0.0, 0.0, 0.0, 0.0)), rand3(v + vec4(0.0, 1.0, 0.0, 0.0)), c.t),
				mix(rand3(v + vec4(1.0, 0.0, 0.0, 0.0)), rand3(v + vec4(1.0, 1.0, 0.0, 0.0)), c.t), c.s);

}


/**
 *  Calculate one sample of 1D Solid Noise.
 *  Return a uniform 3D vector on the unit sphere.
 *
 */
vec3 noise3(float x)
{
	float u = fract(x);
	vec4  v = vec4(x - u, 0.0, 0.0, 0.0);
	float c = curve(u);

	return mix(rand3(v + vec4(0.0, 0.0, 0.0, 0.0)), rand3(v + vec4(1.0, 0.0, 0.0, 0.0)), c);

}


/**
 *  Calculate one sample of 4D fBm noise.
 *  Return a uniform 3D vector on the unit sphere.
 *
 */
vec3 fbm3(vec4 x, float f0, float df, float a0, float da, int N)
{
	vec3  s = vec3(0.0);
	float a = a0, f = f0;

	for (int n=0; n < N; n++) {
		s += a * noise3(f * x);
		a *= da;
		f *= df;
	}

	return s;

}


/**
 *  Calculate one sample of 3D fBm noise.
 *  Return a uniform 3D vector on the unit sphere.
 *
 */
vec3 fbm3(vec3 x, float f0, float df, float a0, float da, int N)
{
	vec3  s = vec3(0.0);
	float a = a0, f = f0;

	for (int n=0; n < N; n++) {
		s += a * noise3(f * x);
		a *= da;
		f *= df;
	}

	return s;

}


/**
 *  Calculate one sample of 2D fBm noise.
 *  Return a uniform 3D vector on the unit sphere.
 *
 */
vec3 fbm3(vec2 x, float f0, float df, float a0, float da, int N)
{
	vec3  s = vec3(0.0);
	float a = a0, f = f0;

	for (int n=0; n < N; n++) {
		s += a * noise3(f * x);
		a *= da;
		f *= df;
	}

	return s;

}


/**
 *  Calculate one sample of 1D fBm noise.
 *  Return a uniform 3D vector on the unit sphere.
 *
 */
vec3 fbm3(float x, float f0, float df, float a0, float da, int N)
{
	vec3  s = vec3(0.0);
	float a = a0, f = f0;

	for (int n=0; n < N; n++) {
		s += a * noise3(f * x);
		a *= da;
		f *= df;
	}

	return s;

}

