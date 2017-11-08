

#section
#version 450
#extension GL_ARB_separate_shader_objects: enable

#include <gpuio.glsl>

#ifndef _HADES_CORE_GLSL_H
#define _HADES_CORE_GLSL_H


#section .vert

layout(location=GPU_ATTRIBUTE_VERTEX)  in vec4 g_vertex;
layout(location=GPU_ATTRIBUTE_TEXTURE) in vec4 g_texcoords;
layout(location=GPU_ATTRIBUTE_COLOR)   in vec4 g_color;
layout(location=GPU_ATTRIBUTE_NORMAL)  in vec4 g_normal;
layout(location=GPU_ATTRIBUTE_EXTRA)   in vec4 g_extra;
layout(location=GPU_ATTRIBUTE_BONE0)   in vec4 g_bones0;
layout(location=GPU_ATTRIBUTE_BONE1)   in vec4 g_bones1;
layout(location=GPU_ATTRIBUTE_BONE2)   in vec4 g_bones2;
layout(location=GPU_ATTRIBUTE_BONE3)   in vec4 g_bones3;

out gl_PerVertex { //FIXME: Make this customizable
    vec4 gl_Position;
};

#define VARIABLE(x) layout(location=x) out


#section .frag

#ifdef GL_ES
precision mediump float;
#endif

#define VARIABLE(x) layout(location=x) in
#define OUTPUT(x)   layout(location=x) out


#section
#endif

