#pragma once
#include "common.h"

char* readShader(const char *filename);

GLuint loadShader(const char *filename, GLenum shaderType);

void loadProgram(GLuint program, const char *vsPath, const char *fsPath);

GLint findUniform(GLuint program, const char *uniform);

void initializeProgramSampler(GLuint program, const char *sampler, s32 unit);
