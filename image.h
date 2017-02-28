#pragma once
#include "common.h"

// Image

typedef struct {
    u32 xres, yres;
    unsigned char* pixels;
} Image;

Image loadPng(const char *filename);

// Texture

typedef struct {
    vec2 size;
    GLuint id;
} Texture;

Texture loadTexture(const char* filename);

typedef struct {
    Texture texture;
    s32     currFrame, nFrames;
    f32     oneOverNFrames;
} TextureAnim;
