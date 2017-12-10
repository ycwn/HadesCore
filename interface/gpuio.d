

import hades.core_types;


//enum GPU_UNIFORM_FRAME    = 0;
enum GPU_UNIFORM_TEXTURES = 0;
enum GPU_UNIFORM_SHADER   = 1;
enum GPU_UNIFORM_OBJECT   = 2;
enum GPU_UNIFORM_MATERIAL = 3;
enum GPU_UNIFORM_SKELETON = 4;

enum GPU_ATTRIBUTE_VERTEX  = 0;
enum GPU_ATTRIBUTE_TEXTURE = 1;
enum GPU_ATTRIBUTE_COLOR   = 2;
enum GPU_ATTRIBUTE_TANGENT = 3;
enum GPU_ATTRIBUTE_EXTRA   = 4;
enum GPU_ATTRIBUTE_BONE0   = 5;
enum GPU_ATTRIBUTE_BONE1   = 6;
enum GPU_ATTRIBUTE_BONE2   = 7;
enum GPU_ATTRIBUTE_BONE3   = 8;

enum GPU_TEXTURE_2D_BINDING   = 0;
enum GPU_TEXTURE_3D_BINDING   = 1;
enum GPU_TEXTURE_CUBE_BINDING = 2;


enum GPU_SHADER_ARGUMENTS   = 15;
enum GPU_OBJECT_ARGUMENTS   = 4;
enum GPU_MATERIAL_TEXTURES  = 16;
enum GPU_MATERIAL_ARGUMENTS = 7;
enum GPU_SKELETON_BONES     = 256;
enum GPU_TEXTURES_2D        = 10240;
enum GPU_TEXTURES_3D        = 1024;
enum GPU_TEXTURES_CUBE      = 2048;


enum GPU_TEXTURES_MIN = 0;
enum GPU_TEXTURES_NUM = GPU_TEXTURES_2D + GPU_TEXTURES_3D + GPU_TEXTURES_CUBE;

enum GPU_TEXTURES_2D_BEGIN = GPU_TEXTURES_MIN;
enum GPU_TEXTURES_2D_END   = GPU_TEXTURES_2D_BEGIN + GPU_TEXTURES_2D;

enum GPU_TEXTURES_3D_BEGIN = GPU_TEXTURES_2D_END;
enum GPU_TEXTURES_3D_END   = GPU_TEXTURES_3D_BEGIN + GPU_TEXTURES_3D;

enum GPU_TEXTURES_CUBE_BEGIN = GPU_TEXTURES_3D_END;
enum GPU_TEXTURES_CUBE_END   = GPU_TEXTURES_CUBE_BEGIN + GPU_TEXTURES_CUBE;



struct gpu_uniform_frame {

	vec4 time;   // time, delta, delta^2, epoch

}


struct gpu_uniform_shader {

	vec4 screen;  // width, height, hcoeff, vcoeff

	vec4[GPU_SHADER_ARGUMENTS] arg; // Free shader parameters

}


struct gpu_uniform_object {

	mat4 mat_model;      // ModelSpace to WorldSpace
	mat4 mat_view;       // WorldSpace to EyeSpace
	mat4 mat_proj;       // EyeSpace to Device
	mat4 mat_eye;        // EyeSpace to WorldSpace
	mat4 mat_modelview;  // ModelSpace to EyeSpace
	mat4 mat_viewproj;   // WorldSpace to Device
	mat4 mat_mvp;        // ModelSpace to Device

	vec4[GPU_OBJECT_ARGUMENTS] arg; // Free object parameters

}


struct gpu_uniform_material {

	vec4 minimum;  // ModelSpace minimum extents
	vec4 maximum;  // ModelSpace maximum extents

	vec4 albedo;
	vec4 emission;
	vec4 material;

	uint[GPU_MATERIAL_TEXTURES] textures; // Texture IDs

	vec4[GPU_MATERIAL_ARGUMENTS] arg; // Free material parameters

}


struct gpu_uniform_skeleton {

	mat4[GPU_SKELETON_BONES] arg; // Skeletal bones

}

