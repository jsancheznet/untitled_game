#include "renderer.h"

u32 Renderer::CompileShader(const char* Filename)
{
    Assert(Filename);

    size_t Size;
    char *FileString = static_cast<char*>(SDL_LoadFile(Filename, &Size));
    
    u32 VertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char *VertexSource[2] = {"#version 460 core\n#define VERTEX_SHADER\n", FileString};
    glShaderSource(VertexShader, 2, VertexSource, NULL);
    glCompileShader(VertexShader);
    i32 Compiled;
    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Compiled);
    if (Compiled != GL_TRUE)
    {
        i32 LogLength = 0;
        char ErrorMessage[1024];
        glGetShaderInfoLog(VertexShader, 1024, &LogLength, ErrorMessage);
        fprintf(stderr, "%s-%s\n", Filename, ErrorMessage);
        VertexShader = 0;
    }


    u32 FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *FragmentSource[2] = {"#version 460 core\n#define FRAGMENT_SHADER\n", FileString};
    glShaderSource(FragmentShader, 2, FragmentSource, NULL);
    glCompileShader(FragmentShader);
    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Compiled);
    if (Compiled != GL_TRUE)
    {
        i32 LogLength = 0;
        char ErrorMessage[1024];
        glGetShaderInfoLog(FragmentShader, 1024, &LogLength, ErrorMessage);
        fprintf(stderr, "%s-%s\n", Filename, ErrorMessage);
        FragmentShader = 0;
    }

    u32 CompiledShader = glCreateProgram();
    glAttachShader(CompiledShader, VertexShader);
    glAttachShader(CompiledShader, FragmentShader);
    glLinkProgram(CompiledShader);
    i32 IsLinked = 0;
    glGetProgramiv(CompiledShader, GL_LINK_STATUS, (GLint *)&IsLinked);
    if (IsLinked == GL_FALSE)
    {
        i32 MaxLogLength = 1024;
        char InfoLog[1024] = {0};
        glGetProgramInfoLog(CompiledShader, MaxLogLength, &MaxLogLength, &InfoLog[0]);
        printf("%s: SHADER PROGRAM FAILED TO COMPILE/LINK\n", Filename);
        printf("%s\n", InfoLog);
        glDeleteProgram(CompiledShader);
        CompiledShader = 0;
    }

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
    SDL_free(FileString);

    return CompiledShader;
}

void Renderer::OpenGLDebugMessageCallback(GLenum Source, GLenum Type, GLuint Id, GLenum Severity, GLsizei Length, GLchar const* Message, void const* UserParam)
{
    Source;
    Type;
    Id;
    Severity;
    Length;
    Message;
    UserParam;

    switch (Source)
    {
        case GL_DEBUG_SOURCE_API: printf("API:"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: printf("WINDOW SYSTEM:"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: printf("SHADER COMPILER:"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: printf("THIRD PARTY:"); break;
        case GL_DEBUG_SOURCE_APPLICATION: printf("APPLICATION:"); break;
        case GL_DEBUG_SOURCE_OTHER: printf("OTHER:"); break;
    }

    switch (Type)
    {
        case GL_DEBUG_TYPE_ERROR: printf("ERROR:"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: printf("DEPRECATED_BEHAVIOR:"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: printf("UNDEFINED_BEHAVIOR:"); break;
        case GL_DEBUG_TYPE_PORTABILITY: printf("PORTABILITY:"); break;
        case GL_DEBUG_TYPE_PERFORMANCE: printf("PERFORMANCE:"); break;
        case GL_DEBUG_TYPE_MARKER: printf("MARKER:"); break;
        case GL_DEBUG_TYPE_OTHER: printf("OTHER:"); break;
    }

    switch (Severity)
    {
        case GL_DEBUG_SEVERITY_NOTIFICATION: printf("NOTIFICATION:"); break;
        case GL_DEBUG_SEVERITY_LOW: printf("LOW:"); break;
        case GL_DEBUG_SEVERITY_MEDIUM: printf("MEDIUM:"); break;
        case GL_DEBUG_SEVERITY_HIGH: printf("HIGH:"); break;
    }

    printf("->  %s\n", Message);    
}