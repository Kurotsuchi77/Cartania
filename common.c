#include "common.h"
#include <math.h>
#include <stdlib.h>

// Utils

u32 randInRange(u32 min, u32 max)
{
    return rand() % (max + 1 - min) + min;
}

// vec2

vec2 v2New(f32 x, f32 y)
{
    vec2 v = { x, y };
    return  v;
}

vec2 v2Add(vec2 a, vec2 b)
{
    return v2New(a.x + b.x, a.y + b.y);
}

vec2 v2Sub(vec2 a, vec2 b)
{
    return v2New(a.x - b.x, a.y - b.y);
}

vec2 v2Mul(vec2 a, vec2 b)
{
    return v2New(a.x * b.x, a.y * b.y);
}

vec2 v2Mulf(vec2 v, f32 f)
{
    return v2New(v.x * f, v.y * f);
}

bool v2Equal(vec2 a, vec2 b)
{
    return
        (a.x - EPSILON < b.x && a.x + EPSILON > b.x) &&
        (a.y - EPSILON < b.y && a.y + EPSILON > b.y);
}

f32 v2Len(vec2 v)
{
    return sqrt(v.x*v.x + v.y*v.y);
}

vec2 v2Normalize(vec2 v)
{
    f32 len = v2Len(v);
    return v2New(v.x / len, v.y / len);
}

// vec4

vec4 v4New(f32 x, f32 y, f32 z, f32 w)
{
    vec4 v = { x, y, z, w };
    return  v;
}

