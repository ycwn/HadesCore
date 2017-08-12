
#section
#ifndef _ORION_CORE_GLSL_H
#define _ORION_CORE_GLSL_H


#section .vert

attribute vec4 g_vertex;
attribute vec4 g_texcoords;
attribute vec4 g_color;
attribute vec4 g_normal;
attribute vec4 g_tangent;
attribute vec4 g_extra;
attribute vec4 g_bones0;
attribute vec4 g_bones1;
attribute vec4 g_bones2;
attribute vec4 g_bones3;


#section .frag

#ifdef GL_ES
precision mediump float;
#endif


#section .vert, .frag

uniform vec4 g_screen;  // width, height, hcoeff, vcoeff
uniform vec4 g_timer;   // time, delta, delta^2, epoch

uniform mat4 g_mat_model;      // ModelSpace to WorldSpace
uniform mat4 g_mat_normal;     // ModelSpace to WorldSpace (unscaled)
uniform mat4 g_mat_view;       // WorldSpace to EyeSpace
uniform mat4 g_mat_proj;       // EyeSpace to Device
uniform mat4 g_mat_eye;        // EyeSpace to WorldSpace
uniform mat4 g_mat_modelview;  // ModelSpace to EyeSpace
uniform mat4 g_mat_viewproj;   // WorldSpace to Device
uniform mat4 g_mat_mvp;        // ModelSpace to Device

uniform vec4  g_eye_position;  // Camera position in WorldSpace
uniform float g_eye_distance;  // Distance of Camera to ModelSpace origin
uniform float g_eye_exposure;  // Exposure value for tone mapping
uniform float g_eye_gamma;     // Gamma ramp

uniform vec4 g_frag_v0; // Free fragment parameters
uniform vec4 g_frag_v1;
uniform vec4 g_frag_v2;
uniform vec4 g_frag_v3;
uniform vec4 g_frag_v4;
uniform vec4 g_frag_v5;
uniform vec4 g_frag_v6;
uniform vec4 g_frag_v7;
uniform vec4 g_frag_v8;
uniform vec4 g_frag_v9;
uniform vec4 g_frag_v10;
uniform vec4 g_frag_v11;
uniform vec4 g_frag_v12;
uniform vec4 g_frag_v13;
uniform vec4 g_frag_v14;
uniform vec4 g_frag_v15;

uniform vec4 g_light_ambient; // Global ambience

uniform vec4  g_light0_position; // Position of Light0 in WorldSpace
uniform vec4  g_light0_diffuse;  // Diffuse color of Light0
uniform vec4  g_light0_specular; // Specular color of Light0
uniform vec4  g_light0_attr;     // Effective radius, brightness, 0.0, 0.0
uniform float g_light0_shadow;   // ShadowMap parameters of Light0: Unknown, to be implemented

uniform vec4  g_light1_position;
uniform vec4  g_light1_diffuse;
uniform vec4  g_light1_specular;
uniform vec4  g_light1_attr;
uniform float g_light1_shadow;

uniform vec4  g_light2_position;
uniform vec4  g_light2_diffuse;
uniform vec4  g_light2_specular;
uniform vec4  g_light2_attr;
uniform float g_light2_shadow;

uniform vec4  g_light3_position;
uniform vec4  g_light3_diffuse;
uniform vec4  g_light3_specular;
uniform vec4  g_light3_attr;
uniform float g_light3_shadow;

/**
 * NVIDIA doesn't cull unused samplers on Tegra3,
 * and ATI must have proper values assigned (even
 * if unused!) or the driver crashes.
 */
#if ENABLE_SAMPLER_TEX0_2D
uniform sampler2D g_tex0_2d; // 2D texture units
#endif

#if ENABLE_SAMPLER_TEX1_2D
uniform sampler2D g_tex1_2d;
#endif

#if ENABLE_SAMPLER_TEX2_2D
uniform sampler2D g_tex2_2d;
#endif

#if ENABLE_SAMPLER_TEX3_2D
uniform sampler2D g_tex3_2d;
#endif

#if ENABLE_SAMPLER_TEX4_2D
uniform sampler2D g_tex4_2d;
#endif

#if ENABLE_SAMPLER_TEX5_2D
uniform sampler2D g_tex5_2d;
#endif

#if ENABLE_SAMPLER_TEX6_2D
uniform sampler2D g_tex6_2d;
#endif

#if ENABLE_SAMPLER_TEX7_2D
uniform sampler2D g_tex7_2d;
#endif

#if ENABLE_SAMPLER_TEX0_CUBE
uniform samplerCube g_tex0_cube; // Cube texture units
#endif

#if ENABLE_SAMPLER_TEX1_CUBE
uniform samplerCube g_tex1_cube;
#endif

#if ENABLE_SAMPLER_TEX2_CUBE
uniform samplerCube g_tex2_cube;
#endif

#if ENABLE_SAMPLER_TEX3_CUBE
uniform samplerCube g_tex3_cube;
#endif

#if ENABLE_SAMPLER_TEX4_CUBE
uniform samplerCube g_tex4_cube;
#endif

#if ENABLE_SAMPLER_TEX5_CUBE
uniform samplerCube g_tex5_cube;
#endif

#if ENABLE_SAMPLER_TEX6_CUBE
uniform samplerCube g_tex6_cube;
#endif

#if ENABLE_SAMPLER_TEX7_CUBE
uniform samplerCube g_tex7_cube;
#endif

#if ENABLE_SAMPLER_LIGHT_SHADOWMAP
uniform sampler2D g_light_shadowmap;  // Global shadowmap
#endif

#if ENABLE_SAMPLER_LIGHT0_PROJ
uniform samplerCube g_light0_proj; // Projection cubemap
#endif

#if ENABLE_SAMPLER_LIGHT1_PROJ
uniform samplerCube g_light1_proj;
#endif

#if ENABLE_SAMPLER_LIGHT2_PROJ
uniform samplerCube g_light2_proj;
#endif

#if ENABLE_SAMPLER_LIGHT3_PROJ
uniform samplerCube g_light3_proj;
#endif

#if ENABLE_SAMPLER_LIGHT0_SPOT
uniform sampler2D g_light0_spot; // Projection spotlight for Light0
#endif

#if ENABLE_SAMPLER_LIGHT1_SPOT
uniform sampler2D g_light1_spot;
#endif

#if ENABLE_SAMPLER_LIGHT2_PROJ
uniform sampler2D g_light2_spot;
#endif

#if ENABLE_SAMPLER_LIGHT3_PROJ
uniform sampler2D g_light3_spot;
#endif


#section
#endif

