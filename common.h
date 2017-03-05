#pragma once


#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdbool.h>

#define TRUE    1
#define FALSE   0
#define EPSILON 0.000005f

// Types

typedef signed int      s32;
typedef unsigned int    u32;
typedef float           f32;
typedef double          f64;

// Utils

u32 randInRange(u32 min, u32 max);

// vec2

typedef struct {
    f32 x, y;
} vec2;

vec2 v2New(f32 x, f32 y);

vec2 v2Add(vec2 a, vec2 b);

vec2 v2Sub(vec2 a, vec2 b);

vec2 v2Mul(vec2 a, vec2 b);

vec2 v2Mulf(vec2 v, f32 f);

bool v2Equal(vec2 a, vec2 b);

f32 v2Len(vec2 v);

vec2 v2Normalize(vec2 v);

// vec4

typedef struct {
    f32 x, y, z, w;
} vec4;

vec4 v4New(f32 x, f32 y, f32 z, f32 w);
