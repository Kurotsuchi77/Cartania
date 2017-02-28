#pragma once
#include "common.h"

// Font

typedef struct
{
    struct {
        f32 ax, ay;
        f32 bw, bh;
        f32 bl, bt;
        f32 tx;
    } characters[128];
    u32 atlasx, atlasy;
    f32 sizey;
    GLuint charTex;
} Font;

Font loadFont(const char *filename, s32 sizey, f32 ary);

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
} Align;

typedef struct {
    Font *pFont;
    Align align;

    f32 anchorPosX;
    vec2 pos;
    vec4 color;

    GLuint vao, vbo, nVertices;
    GLuint charTex;
} Label;

void buildLabel(vec2 ratios, Font *pFont, const char *str, Label *pLabel);

Label newLabel(vec2 ratios, Font *pFont, const char *str, Align align, vec2 pos, vec4 color);

Label newIntegerLabel(vec2 ratios, Font *pFont, s32 i, Align align, vec2 pos, vec4 color);

void setLabelStr(vec2 ratios, Font* pFont, Label* pLabel, const char* str);
