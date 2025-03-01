#include <iostream>

#include <SDL3/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderer.h"
#include "typedefs.h"

// TODO: Do a little refactor, Move functions into Renderer, Renderer.Init(); Renderer.Update(), Renderer.Draw(); ?? These are only examplesa

// TODO: Handle Window Resize, Quad should be in center no matter the resize
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

    // TODO: Platform->CreateWindow("Untitled", 1366, 768);
    SDL_Window* Window = SDL_CreateWindow("Untitled", 1366, 768, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    SDL_GLContext GLContext = SDL_GL_CreateContext(Window);

    Renderer *Render = new Renderer;
    Render->Init();

    // TODO(Jsanchez): Create a Camera file!, it might be the best place to put this at!
        
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
        
        Render->BeginFrame();

        i32 ModelLocation = glGetUniformLocation(Render->Shader, "Model");
        i32 ViewLocation = glGetUniformLocation(Render->Shader, "View");
        i32 ProjectionLocation = glGetUniformLocation(Render->Shader, "Projection");

        glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, glm::value_ptr(Model));
        glUniformMatrix4fv(ViewLocation, 1, GL_FALSE, glm::value_ptr(View));
        glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, glm::value_ptr(Projection));
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

       SDL_GL_SwapWindow(Window); // TODO(Jsanchez): Move Window to the Renderer? Maybe a copy of it?, i might need it for EndFrame()
    }
    
    SDL_Quit();
}