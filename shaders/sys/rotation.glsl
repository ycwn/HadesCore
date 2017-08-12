
#section
#ifndef _ORION_ROTATION_GLSL_H
#define _ORION_ROTATION_GLSL_H


#section .vert, .frag

mat3 xrotate3(float t)
{

	float s = sin(t);
	float c = cos(t);

	return mat3(
		1.0, 0.0, 0.0,
		0.0,   c,  -s,
		0.0,   s,   c);

}



mat3 yrotate3(float t)
{

	float s = sin(t);
	float c = cos(t);

	return mat3(
		  c, 0.0,   s,
		0.0, 1.0, 0.0,
		 -s, 0.0,   c);

}



mat3 zrotate3(float t)
{

	float s = sin(t);
	float c = cos(t);

	return mat3(
		  c,  -s, 0.0,
		  s,   c, 0.0,
		0.0, 0.0, 0.0);

}



mat3 rotate3(vec3 v, float t)
{

	float s = sin(t);
    float c = cos(t);
	float oc = 1.0 - c;

	return mat3(
		oc * v.x * v.x + c,       oc * v.x * v.y - v.z * s, oc * v.z * v.x + v.y * s,
		oc * v.x * v.y + v.z * s, oc * v.y * v.y + c,       oc * v.y * v.z - v.x * s,
		oc * v.z * v.x - v.y * s, oc * v.y * v.z + v.x * s, oc * v.z * v.z + c);

}



mat4 xrotate4(float t)
{

	float s = sin(t);
	float c = cos(t);

	return mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0,   c,  -s, 0.0,
		0.0,   s,   c, 0.0,
		0.0, 0.0, 0.0, 1.0);

}



mat4 yrotate4(float t)
{

	float s = sin(t);
	float c = cos(t);

	return mat4(
		  c, 0.0,   s, 0.0,
		0.0, 1.0, 0.0, 0.0,
		 -s, 0.0,   c, 0.0,
		0.0, 0.0, 0.0, 1.0);

}



mat4 zrotate4(float t)
{

	float s = sin(t);
	float c = cos(t);

	return mat4(
		  c,  -s, 0.0, 0.0,
		  s,   c, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 1.0);

}



mat4 rotate4(vec3 v, float t)
{

	float s = sin(t);
	float c = cos(t);
	float oc = 1.0 - c;

	return mat4(
		oc * v.x * v.x + c,       oc * v.x * v.y - v.z * s, oc * v.z * v.x + v.y * s, 0.0,
		oc * v.x * v.y + v.z * s, oc * v.y * v.y + c,       oc * v.y * v.z - v.x * s, 0.0,
		oc * v.z * v.x - v.y * s, oc * v.y * v.z + v.x * s, oc * v.z * v.z + c,       0.0,
		0.0,                      0.0,                         0.0,                   1.0);

}


#section
#endif
