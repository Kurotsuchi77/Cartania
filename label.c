#include "label.h"
#include <ft2build.h>
#include FT_FREETYPE_H

Font loadFont(const char *filename, s32 sizey, f32 ary)
{
    Font font;
    font.atlasx = font.atlasy = 0;

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        puts("Error initializing freetype library");
        exit(EXIT_FAILURE);
    }

    FT_Face face;
    if (FT_New_Face(ft, filename, 0, &face)) {
        printf("Error opening font %s\n", filename);
        exit(EXIT_FAILURE);
    }

    FT_Set_Pixel_Sizes(face, 0, sizey);
    FT_GlyphSlot g = face->glyph;

    font.sizey = (f32)sizey * ary;

    for (s32 i = 32; i < 128; i++)
    {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            printf("Font %s : failed to load character %c\n", filename, i);
            continue;
        }

        font.atlasx += g->bitmap.width;
        font.atlasy = font.atlasy > g->bitmap.rows ? font.atlasy : g->bitmap.rows;
    }

    GLint align;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &align);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unsigned char pixels[font.atlasx*font.atlasy];
    for (s32 i = 0; i < font.atlasx*font.atlasy; i++)
        pixels[i] = 0;

    glGenTextures(1, &font.charTex);
    glBindTexture(GL_TEXTURE_2D, font.charTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font.atlasx, font.atlasy, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    s32 x = 0;
    for (s32 i = 32; i < 128; i++)
    {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER))
            continue;

        glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

        font.characters[i].ax = g->advance.x >> 6;
        font.characters[i].ay = g->advance.y >> 6;

        font.characters[i].bw = g->bitmap.width;
        font.characters[i].bh = g->bitmap.rows;

        font.characters[i].bl = g->bitmap_left;
        font.characters[i].bt = g->bitmap_top;

        font.characters[i].tx = (f32)x / (f32)font.atlasx;

        x += g->bitmap.width;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, align);

    return font;
}


void buildLabel(vec2 ratios, Font *pFont, const char *str, Label *pLabel)
{
    typedef struct {
        f32 x, y;
        f32 s, t;
    } Vertex;

    Vertex vertices[6 * strlen(str)];

    f32 x = 0.0f, y = 0.0f;
    s32 n = 0;

    for (const char *p = str; *p; p++)
    {
        if (*p == '\n') {
            x  = 0.0f;
            y -= pFont->sizey;
            continue;
        }

        f32 x2 =  x + pFont->characters[*p].bl * ratios.x;
        f32 y2 = -y - pFont->characters[*p].bt * ratios.y;
        f32 w = pFont->characters[*p].bw * ratios.x;
        f32 h = pFont->characters[*p].bh * ratios.y;

        // Advance the cursor to the start of the next character
        x += pFont->characters[*p].ax * ratios.x;
        y += pFont->characters[*p].ay * ratios.y;

        // Skip glyphs that have no pixels
        if (!w || !h) continue;

        vertices[n++] = (Vertex){x2, -y2 , pFont->characters[*p].tx, 0};
        vertices[n++] = (Vertex){x2, -y2 - h, pFont->characters[*p].tx, pFont->characters[*p].bh / pFont->atlasy};
        vertices[n++] = (Vertex){x2 + w, -y2 , pFont->characters[*p].tx + pFont->characters[*p].bw / pFont->atlasx, 0};
        vertices[n++] = (Vertex){x2 + w, -y2 , pFont->characters[*p].tx + pFont->characters[*p].bw / pFont->atlasx, 0};
        vertices[n++] = (Vertex){x2, -y2 - h, pFont->characters[*p].tx, pFont->characters[*p].bh / pFont->atlasy};
        vertices[n++] = (Vertex){x2 + w, -y2 - h, pFont->characters[*p].tx + pFont->characters[*p].bw / pFont->atlasx, pFont->characters[*p].bh / pFont->atlasy};
    }

    if      (pLabel->align == ALIGN_CENTER) pLabel->anchorPosX = pLabel->pos.x - x / 2.0f;
    else if (pLabel->align == ALIGN_RIGHT ) pLabel->anchorPosX = pLabel->pos.x - x;
    else pLabel->anchorPosX = pLabel->pos.x;

    glGenBuffers(1, &pLabel->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pLabel->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    pLabel->nVertices = n;
}

Label newLabel(vec2 ratios, Font *pFont, const char *str, Align align, vec2 pos, vec4 color)
{
    Label label;
    label.pFont     = pFont;
    label.align     = align;
    label.pos       = pos;
    label.color     = color;
    label.charTex   = pFont->charTex;

    glGenVertexArrays(1, &label.vao);
    glBindVertexArray(label.vao);

    buildLabel(ratios, pFont, str, &label);

    return label;
}

Label newIntegerLabel(vec2 ratios, Font *pFont, s32 i, Align align, vec2 pos, vec4 color)
{
    char str[3];
    snprintf(str, 3, "%d", i);

    return newLabel(ratios, pFont, str, align, pos, color);
}

void setLabelStr(vec2 ratios, Font *pFont, Label *pLabel, const char *str)
{
    glDeleteBuffers(1, &pLabel->vbo);
    glBindVertexArray(pLabel->vao);
    buildLabel(ratios, pFont, str, pLabel);
}
