#pragma once

#include "typedefs.h"
#include <glad/glad.h>


class Renderer
{
public:
    
    u32 VAO = 0;
    u32 VBO = 0;
    u32 EBO = 0;

    u32 Shader = 0;
    u32 MyTexture = 0;

    void Init();

    void BeginFrame();
    
    u32 CompileShader(const char* Filename);
    u32 CreateTexture(const char* Filepath);
    
private:
    
    static void OpenGLDebugMessageCallback(GLenum Source, GLenum Type, GLuint Id, GLenum Severity, GLsizei Length, GLchar const* Message, void const* UserParam);
};