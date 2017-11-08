

#section
#ifndef _HADES_GPUIO_GLSL_H
#define _HADES_GPUIO_GLSL_H


#define GPU_UNIFORM_FRAME   0
#define GPU_UNIFORM_SHADER  0
#define GPU_UNIFORM_OBJECT  1

#define GPU_ATTRIBUTE_VERTEX   0
#define GPU_ATTRIBUTE_TEXTURE  1
#define GPU_ATTRIBUTE_COLOR    2
#define GPU_ATTRIBUTE_NORMAL   3
#define GPU_ATTRIBUTE_EXTRA    4
#define GPU_ATTRIBUTE_BONE0    5
#define GPU_ATTRIBUTE_BONE1    6
#define GPU_ATTRIBUTE_BONE2    7
#define GPU_ATTRIBUTE_BONE3    8

#define GPU_SHADER_ARGUMENTS  16
#define GPU_OBJECT_ARGUMENTS  16


/*
layout(binding=0, set=GPU_UNIFORM_FRAME) uniform g_frame_t {

	vec4 time;   // time, delta, delta^2, epoch

} g_frame;
*/

layout(binding=0, set=GPU_UNIFORM_SHADER) uniform g_shader_t {

	vec4 screen;  // width, height, hcoeff, vcoeff

	vec4 arg[GPU_SHADER_ARGUMENTS]; // Free shader parameters

} g_shader;


layout(binding=0, set=GPU_UNIFORM_OBJECT) uniform g_object_t {

	mat4 mat_model;      // ModelSpace to WorldSpace
	mat4 mat_normal;     // ModelSpace to WorldSpace (unscaled)
	mat4 mat_view;       // WorldSpace to EyeSpace
	mat4 mat_proj;       // EyeSpace to Device
	mat4 mat_eye;        // EyeSpace to WorldSpace
	mat4 mat_modelview;  // ModelSpace to EyeSpace
	mat4 mat_viewproj;   // WorldSpace to Device
	mat4 mat_mvp;        // ModelSpace to Device

	vec4 eye_position;  // xyz: Camera position in WorldSpace, w: Distance of Camera to ModelSpace origin
	vec4 eye_exposure;  // x: Exposure value for tone mapping, y: Gamma ramp

	vec4 arg[GPU_OBJECT_ARGUMENTS]; // Free object parameters

} g_object;


#section
#endif

