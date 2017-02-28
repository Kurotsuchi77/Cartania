#include "image.h"
#include <stdlib.h>
#include "lodepng.h"

// Image

Image loadPng(const char* filename)
{
    Image image;

    u32 error = lodepng_decode32_file(&image.pixels, &image.xres, &image.yres, filename);
    if (error) {
        printf("Error decoding PNG image %s : %s\n", filename, lodepng_error_text(error));
        exit(EXIT_FAILURE);
    }

    return image;
}

// Texture

Texture loadTexture(const char *filename)
{
    Image image = loadPng(filename);

    Texture texture;

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.xres, image.yres, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    texture.size = v2New((f32)image.xres, (f32)image.yres);

    free(image.pixels);

    return texture;
}
