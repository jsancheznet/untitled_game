#include <iostream>

#include <SDL3/SDL.h>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "renderer.h"
#include "typedefs.h"

// TODO: Handle Window Resize, Quad should be in center no matter the resize
// TODO: Do a little refactor, Move functions into Renderer, Renderer.Init(); Renderer.Update(), Renderer.Draw(); ?? These are only examplesa
// TODO: Research RK Integrator and do i need one of those? or something similar? Verlet Integration? Which one is Casey's?
// TODO: Integrar IMGUI

// BIG TASKS
//    - RK Integration?
//        - Reread and research what this means and how to do it
//    - Fast Text Rendering
//        - I can do this using a spritefont sheet!, it can be the same implementation as a spritesheet and kill two birds with one stone
//        - Research how this is done! Find something i can implement
//    - Spritesheet Animation
//         - Research 

// RIDER TODO
// - Mejorar el estado de los includes, incluye cualquier cosa y no es bueno
// - A veces el autocomplete molesta mas que ayuda

// NOTES
//     - Quadtree might be a good algorithm for partioning my world!

b32 IsRunning = true;

f32 QuadPositionX = 0.0f;

void ProcessEvents()
{
    SDL_Event Event;
    while (SDL_PollEvent(&Event))
    {
        switch (Event.type)
        {
            case SDL_EVENT_QUIT:
            {
                IsRunning = false;
                break;
            }
            case SDL_EVENT_KEY_DOWN:
            {
                if (Event.key.key == SDLK_ESCAPE)
                {
                    IsRunning = false;
                }

                if(Event.key.key == SDLK_RIGHT)
                {
                    QuadPositionX += 0.2f;
                }

                if(Event.key.key == SDLK_LEFT)
                {
                    QuadPositionX -= 0.2f;                    
                }                
                break;
            }
            case SDL_EVENT_KEY_UP:
            {
                break;
            }
            default:
            {
                break;
            }
        }
    }
}

void OpenGLDebugMessageCallback(GLenum Source, GLenum Type, GLuint Id, GLenum Severity, GLsizei Length, GLchar const* Message, void const* UserParam)
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

u32 CreateTexture(const char *Filepath)
{
    u32 Handle;

    i32 Width, Height, ChannelCount;
    stbi_set_flip_vertically_on_load(true);
    u8 *ImageData = stbi_load(Filepath, &Width, &Height, &ChannelCount, 4);

    glCreateTextures(GL_TEXTURE_2D, 1, &Handle);

    glTextureParameteri(Handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(Handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(Handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(Handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTextureStorage2D(Handle, 1, GL_RGBA8, Width, Height);
    
    glTextureSubImage2D(Handle, 0, 0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, ImageData);

    glGenerateTextureMipmap(Handle);
    
    stbi_image_free(ImageData);
    
    return Handle;
}

int main(i32 Argc, char** Argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);    
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    // TODO(Jsanchez): Aliasing, we might be doing pixel art, we may not want aliasing
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    // TODO(Jsanchez): Turn debug off on release
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    SDL_Window* Window = SDL_CreateWindow("Untitled", 1366, 768, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    SDL_GLContext GLContext = SDL_GL_CreateContext(Window);
    
    gladLoadGL();

    { // Enable OpenGL Debug Mode
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(OpenGLDebugMessageCallback, nullptr); // TODO: Call the debug function that's in renderer, not in main
    }
    
    { // Print extensions list to stdout
        // Get the number of extensions
        GLint numExtensions;
        glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

        // Print all extensions
        for (GLint i = 0; i < numExtensions; i++) {
            const char* extensionName = (const char*)glGetStringi(GL_EXTENSIONS, i);
            printf("Extension #%d: %s\n", i, extensionName);
        }
    }

    u32 Shader = Renderer::CompileShader("shaders/hello.glsl");
    u32 MyTexture = CreateTexture("textures/awesomeface.png");

    f32 Vertices[] =
    {
        // Positions        // Texture Coordinates
        0.5f, 0.5f, 0.0f,   1.0f, 1.0f, // top right
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f,  0.0f, 1.0f // top left
    };

    u32 Indices[] =
    {
        0, 1, 3,
        1, 2, 3
    };

    u32 VAO;
    glCreateVertexArrays(1,&VAO);

    u32 VBO;
    glCreateBuffers(1,  &VBO);
    glNamedBufferStorage(VBO, sizeof(Vertices), Vertices, GL_DYNAMIC_STORAGE_BIT);

    u32 EBO;
    glCreateBuffers(1, &EBO);
    glNamedBufferStorage(EBO, sizeof(Indices), Indices, GL_DYNAMIC_STORAGE_BIT);

    // bind vbo and ebo
    glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(f32) * 5); // f32 *5 is the stride between each vertex
    glVertexArrayElementBuffer(VAO, EBO);

    glEnableVertexArrayAttrib(VAO, 0); // Positions
    glEnableVertexArrayAttrib(VAO, 1); // Texture Coordinates

    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0); // Specify the attrib format for the positions
    glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 3); // Specify the attrib format for the texture coordinates

    glVertexArrayAttribBinding(VAO, 0, 0); // I don't understand the last parameter, WTF? Why always 0?
    glVertexArrayAttribBinding(VAO, 1, 0);
    
    glBindTextureUnit(0, MyTexture);

    glm::vec3 CameraPosition = glm::vec3(0.0f, 0.0f, 3.0f); 
    glm::vec3 CameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); 
    glm::vec3 CameraUp = glm::vec3(0.0f, 1.0f, 0.0f); 
    glm::mat4 View = glm::lookAt(CameraPosition, CameraTarget, CameraUp);

    // TODO: What type of projection is better for 2D?
    glm::mat4 Projection = glm::ortho(-240.0f, 240.0f, -135.0f, 135.0f, 0.1f, 100.0f);
    
    while (IsRunning)
    {
        ProcessEvents();

        // Update
        glm::vec3 QuadScale = glm::vec3(80.0f, 80.0, 40.0f);
        glm::vec3 QuadPosition = {QuadPositionX, 0.0f, 0.0f};
        glm::mat4 Model = glm::mat4(1.0f);
        Model = glm::scale(Model, QuadScale);
        Model = glm::translate(Model, QuadPosition);

        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);        
        glUseProgram(Shader);

        i32 ModelLocation = glGetUniformLocation(Shader, "Model");
        i32 ViewLocation = glGetUniformLocation(Shader, "View");
        i32 ProjectionLocation = glGetUniformLocation(Shader, "Projection");

        glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, glm::value_ptr(Model));
        glUniformMatrix4fv(ViewLocation, 1, GL_FALSE, glm::value_ptr(View));
        glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, glm::value_ptr(Projection));
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        SDL_GL_SwapWindow(Window);
    }
    
    SDL_Quit();
}