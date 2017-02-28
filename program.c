#include "program.h"
#include <stdio.h>
#include <stdlib.h>

char* readShader(const char *filename)
{
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error opening shader %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(size + 1);
    if (fread(buffer, 1, size, file) != size) {
        printf("Error reading shader %s\n", filename);
        exit(EXIT_FAILURE);
    }

    buffer[size] = '\0';
    fclose(file);
    return buffer;
}

GLuint loadShader(const char *filename, GLenum shaderType)
{
    char* source = readShader(filename);

    // Compile shader
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const char**)&source, NULL);
    glCompileShader(shader);

    free(source);

    // Check error
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint infoLen;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        GLchar info[infoLen];
        glGetShaderInfoLog(shader, infoLen, NULL, info);
        printf("Error compiling shader %s : %s\n", filename, info);
        exit(EXIT_FAILURE);
    }

    return shader;
}

void loadProgram(GLuint program, const char *vsPath, const char *fsPath)
{
    GLuint vs = loadShader(vsPath, GL_VERTEX_SHADER);
    GLuint fs = loadShader(fsPath, GL_FRAGMENT_SHADER);

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);

    glDetachShader(program, vs);
    glDeleteShader(vs);
    glDetachShader(program, fs);
    glDeleteShader(fs);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint infoLen;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
        GLchar info[infoLen];
        glGetProgramInfoLog(program, infoLen, NULL, info);
        printf("Error linking program %s/%s : %s\n", vsPath, fsPath, info);
        exit(EXIT_FAILURE);
    }

    glUseProgram(program);
}

GLint findUniform(GLuint program, const char *uniform)
{
    GLint loc = glGetUniformLocation(program, uniform);
    if (loc == -1) {
        printf("Error : could not find uniform %s\n", uniform);
    }
    return loc;
}

void initializeProgramSampler(GLuint program, const char *sampler, s32 unit)
{
    glUniform1i(findUniform(program, sampler), unit);
}
