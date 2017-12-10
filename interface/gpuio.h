

#ifndef __INTERFACE_GPUIO_H
#define __INTERFACE_GPUIO_H


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



typedef struct gpu_uniform_frame {

	vec4 time;   // time, delta, delta^2, epoch

} gpu_uniform_frame;


typedef struct gpu_uniform_shader {

	vec4 screen;  // width, height, hcoeff, vcoeff

	vec4 arg[GPU_SHADER_ARGUMENTS]; // Free shader parameters

} gpu_uniform_shader;


typedef struct gpu_uniform_object {

	mat4 mat_model;      // ModelSpace to WorldSpace
	mat4 mat_view;       // WorldSpace to EyeSpace
	mat4 mat_proj;       // EyeSpace to Device
	mat4 mat_eye;        // EyeSpace to WorldSpace
	mat4 mat_modelview;  // ModelSpace to EyeSpace
	mat4 mat_viewproj;   // WorldSpace to Device
	mat4 mat_mvp;        // ModelSpace to Device

	vec4 arg[GPU_OBJECT_ARGUMENTS]; // Free object parameters

} gpu_uniform_object;


typedef struct gpu_uniform_material {

	vec4 minimum;  // ModelSpace minimum extents
	vec4 maximum;  // ModelSpace maximum extents

	vec4 albedo;
	vec4 emission;
	vec4 material;

	uint textures[GPU_MATERIAL_TEXTURES]; // Texture IDs

	vec4 arg[GPU_MATERIAL_ARGUMENTS]; // Free material parameters

} gpu_uniform_material;


typedef struct gpu_uniform_skeleton  {

	mat4 bone[GPU_SKELETON_BONES]; // Skeletal bones

} gpu_uniform_skeleton;


#endif

