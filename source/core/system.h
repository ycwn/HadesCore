
#ifndef __CORE_SYSTEM_H
#define __CORE_SYSTEM_H


#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define VK_NO_PROTOTYPES

#include <vulkan/vulkan.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vectorial/simd4f.h>
#include <vectorial/simd4x4f.h>

typedef simd4f   vec4;
typedef simd4x4f mat4;

#endif

