

#section
#ifndef _HADES_GPUIO_GLSL_H
#define _HADES_GPUIO_GLSL_H


//#define GPU_UNIFORM_FRAME     0
#define GPU_UNIFORM_TEXTURES  0
#define GPU_UNIFORM_SHADER    1
#define GPU_UNIFORM_OBJECT    2
#define GPU_UNIFORM_MATERIAL  3
#define GPU_UNIFORM_SKELETON  4

#define GPU_ATTRIBUTE_VERTEX   0
#define GPU_ATTRIBUTE_TEXTURE  1
#define GPU_ATTRIBUTE_COLOR    2
#define GPU_ATTRIBUTE_TANGENT  3
#define GPU_ATTRIBUTE_EXTRA    4
#define GPU_ATTRIBUTE_BONE0    5
#define GPU_ATTRIBUTE_BONE1    6
#define GPU_ATTRIBUTE_BONE2    7
#define GPU_ATTRIBUTE_BONE3    8

#define GPU_TEXTURE_2D_BINDING    0
#define GPU_TEXTURE_3D_BINDING    1
#define GPU_TEXTURE_CUBE_BINDING  2


#define GPU_SHADER_ARGUMENTS    15
#define GPU_OBJECT_ARGUMENTS    4
#define GPU_MATERIAL_TEXTURES   16
#define GPU_MATERIAL_ARGUMENTS  7
#define GPU_SKELETON_BONES      256
#define GPU_TEXTURES_2D         10240
#define GPU_TEXTURES_3D         1024
#define GPU_TEXTURES_CUBE       2048


#define GPU_TEXTURES_MIN  0
#define GPU_TEXTURES_NUM  (GPU_TEXTURES_2D + GPU_TEXTURES_3D + GPU_TEXTURES_CUBE)

#define GPU_TEXTURES_2D_BEGIN  GPU_TEXTURES_MIN
#define GPU_TEXTURES_2D_END    (GPU_TEXTURES_2D_BEGIN + GPU_TEXTURES_2D)

#define GPU_TEXTURES_3D_BEGIN  GPU_TEXTURES_2D_END
#define GPU_TEXTURES_3D_END    (GPU_TEXTURES_3D_BEGIN + GPU_TEXTURES_3D)

#define GPU_TEXTURES_CUBE_BEGIN  GPU_TEXTURES_3D_END
#define GPU_TEXTURES_CUBE_END    (GPU_TEXTURES_CUBE_BEGIN + GPU_TEXTURES_CUBE)


/*
layout(std140, binding=0, set=GPU_UNIFORM_FRAME) uniform g_frame_t {

	vec4 time;   // time, delta, delta^2, epoch

} g_frame;
*/

layout(binding=GPU_TEXTURE_2D_BINDING,   set=GPU_UNIFORM_TEXTURES) uniform sampler2D   g_surfaces[GPU_TEXTURES_2D];
layout(binding=GPU_TEXTURE_3D_BINDING,   set=GPU_UNIFORM_TEXTURES) uniform sampler3D   g_volumes[GPU_TEXTURES_3D];
layout(binding=GPU_TEXTURE_CUBE_BINDING, set=GPU_UNIFORM_TEXTURES) uniform samplerCube g_cubemaps[GPU_TEXTURES_CUBE];


layout(std140, binding=0, set=GPU_UNIFORM_SHADER) uniform g_shader_t {

	vec4 screen;  // width, height, hcoeff, vcoeff

	vec4 arg[GPU_SHADER_ARGUMENTS]; // Free shader parameters

} g_shader;


layout(std140, binding=0, set=GPU_UNIFORM_OBJECT) uniform g_object_t {

	mat4 mat_model;      // ModelSpace to WorldSpace
	mat4 mat_view;       // WorldSpace to EyeSpace
	mat4 mat_proj;       // EyeSpace to Device
	mat4 mat_eye;        // EyeSpace to WorldSpace
	mat4 mat_modelview;  // ModelSpace to EyeSpace
	mat4 mat_viewproj;   // WorldSpace to Device
	mat4 mat_mvp;        // ModelSpace to Device

	vec4 args[GPU_OBJECT_ARGUMENTS]; // Free object parameters

} g_object;

/*
layout(std140, binding=0, set=GPU_UNIFORM_MATERIAL) uniform g_material_t {

	vec4 minimum;  // ModelSpace minimum extents
	vec4 maximum;  // ModelSpace maximum extents

	vec4 albedo;
	vec4 emission;
	vec4 material;

	uint texture[GPU_MATERIAL_TEXTURES];

	vec4 arg[GPU_MATERIAL_ARGUMENTS]; // Free material parameters

} g_material;


layout(std140, binding=0, set=GPU_UNIFORM_SKELETON) uniform g_skeleton_t {

	mat4 bone[GPU_SKELETON_BONES]; // Skeletal bones

} g_skeleton;
*/

#section
#endif

