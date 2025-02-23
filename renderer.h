#pragma once

#include "typedefs.h"
#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <cstdio>

class Renderer
{
public:
    
    static u32 CompileShader(const char* Filename);

private:

    void OpenGLDebugMessageCallback(GLenum Source, GLenum Type, GLuint Id, GLenum Severity, GLsizei Length, GLchar const* Message, void const* UserParam);
};