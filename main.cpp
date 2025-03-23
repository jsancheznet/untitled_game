#include <fstream>
#include <iostream>


#include <SDL3/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <json.hpp>
#include <sstream>
using json = nlohmann::json;

#include "renderer.h"
#include "typedefs.h"

// - Anotar bien para que sirve cada dato de la font data, se que los atlas bounds son los bounds de la letra en la imagen, en pixeles!
// - Dibujar solo 1 character, sin pensar en batching, usando el json de la font
// - Al procesar la fuente para el renderer hacer todos los calculos una vez sola, ejemplo: Pasar los atlas bounds de pixels a 0..1, y otros calculos mas que se puedan, si no hay que hacerlos cada frame!
// - Hacer que se dibuje bien utilizando SDF
// - Dibujar un string sin pensar en batching
// - Batchear ese string!!

// TODO: Leer un poco sobre sprite rendering
// TODO: Leer un poco sobre batch rendering
// TODO: Window size should not modify aspect ratio
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

struct glyph
{
    i32 UnicodeId;
    f32 Advance;

    // Plane Bounds
    f32 PlaneBoundsLeft;
    f32 PlaneBoundsBottom;
    f32 PlaneBoundsRight;
    f32 PlaneBoundsTop;

    // Atlas Bounds
    f32 AtlasBoundsLeft;
    f32 AtlasBoundsBottom;
    f32 AtlasBoundsRight;
    f32 AtlasBoundsTop;
};

struct kerning_info
{
    i32 Unicode1;
    i32 Unicode2;
    f32 Advance;
};

struct font_data
{
    std::string ImagePath;
    
    // Atlas data
    std::string Type;
    std::string YOrigin;    
    i32 TextureWidth;
    i32 TextureHeight;
    i32 DistanceRange;
    i32 DistanceRangeMiddle;
    f32 Size;

    // Grid Data
    i32 GridCellWidth;
    i32 GridCellHeight;
    i32 GridColumns;
    i32 GridRows;
    f32 GridOriginY;

    // Metrics
    i32 MetricsEmSize;
    f32 MetricsLineHeight;
    f32 MetricsAscender;
    f32 MetricsDescender;
    f32 MetricsUnderlineY;
    f32 MetricsThickness;

    // Glyphs
    std::map<i32, glyph> Glyphs;

    // Kerning Data
    std::vector<kerning_info> KerningInfo;
};

font_data LoadFontData(std::string FontJson, std::string ImagePath)
{
    // TODO(Jsanchez): Add error handling
    
    font_data Result = {};

    Result.ImagePath = ImagePath;
    
    std::ifstream FileStream(FontJson);
    std::ostringstream Buffer;
    Buffer << FileStream.rdbuf();
    std::string JsonString = Buffer.str().c_str();
    
    json Json = json::parse(JsonString);

    // Atlas
    Result.Type = Json["atlas"]["type"].get<std::string>();
    Result.TextureWidth = Json["atlas"]["width"].get<i32>();
    Result.TextureHeight = Json["atlas"]["height"].get<i32>();
    Result.DistanceRange = Json["atlas"]["distanceRange"].get<i32>();
    Result.DistanceRangeMiddle = Json["atlas"]["distanceRangeMiddle"].get<i32>();
    Result.Size = Json["atlas"]["size"].get<f32>();
    Result.YOrigin = Json["atlas"]["yOrigin"].get<std::string>();

    // Grid
    if(Json.contains("grid"))
    {
        Result.GridCellWidth = Json["atlas"]["grid"]["cellWidth"].get<i32>();
        Result.GridCellHeight = Json["atlas"]["grid"]["cellHeight"].get<i32>();
        Result.GridColumns = Json["atlas"]["grid"]["columns"].get<i32>();
        Result.GridRows = Json["atlas"]["grid"]["rows"].get<i32>();
        Result.GridOriginY = Json["atlas"]["grid"]["originY"].get<f32>();
    }

    // Metrics
    Result.MetricsEmSize = Json["metrics"]["emSize"].get<i32>();
    Result.MetricsLineHeight = Json["metrics"]["lineHeight"].get<f32>();
    Result.MetricsAscender = Json["metrics"]["ascender"].get<f32>();
    Result.MetricsDescender = Json["metrics"]["descender"].get<f32>(); 
    Result.MetricsUnderlineY= Json["metrics"]["underlineY"].get<f32>();
    Result.MetricsThickness = Json["metrics"]["underlineThickness"].get<f32>();

    // Glyphs
    i32 GlyphCount = Json["glyphs"].size();
    for(int  i = 0; i < GlyphCount; ++i)
    {
        glyph Glyph = {};
        
        Glyph.UnicodeId = Json["glyphs"][i]["unicode"].get<i32>();
        Glyph.Advance = Json["glyphs"][i]["advance"].get<f32>();

        if(Json["glyphs"][i].contains("planeBounds"))
        {
            Glyph.PlaneBoundsLeft = Json["glyphs"][i]["planeBounds"]["left"].get<f32>();
            Glyph.PlaneBoundsBottom = Json["glyphs"][i]["planeBounds"]["bottom"].get<f32>();
            Glyph.PlaneBoundsRight = Json["glyphs"][i]["planeBounds"]["right"].get<f32>();
            Glyph.PlaneBoundsTop = Json["glyphs"][i]["planeBounds"]["top"].get<f32>();
        }

        if(Json["glyphs"][i].contains("atlasBounds"))
        {
            Glyph.AtlasBoundsLeft = Json["glyphs"][i]["atlasBounds"]["left"].get<f32>();        
            Glyph.AtlasBoundsBottom = Json["glyphs"][i]["atlasBounds"]["bottom"].get<f32>();        
            Glyph.AtlasBoundsRight = Json["glyphs"][i]["atlasBounds"]["right"].get<f32>();        
            Glyph.AtlasBoundsTop = Json["glyphs"][i]["atlasBounds"]["top"].get<f32>();        
        }

        Result.Glyphs[Glyph.UnicodeId] = Glyph;
    }

    i32 KerningCount = Json["kerning"].size();
    if(KerningCount > 0)
    {
        for(i32 i = 0; i < KerningCount; ++i)
        {
            kerning_info KerningInfo = {};

            KerningInfo.Unicode1 = Json["kerning"][i]["unicode1"].get<i32>();
            KerningInfo.Unicode2 = Json["kerning"][i]["unicode2"].get<i32>();
            KerningInfo.Advance = Json["kerning"][i]["advance"].get<f32>();
        }
    }
    
    return Result;
}

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
            
            case SDL_EVENT_WINDOW_RESIZED:
            {
                i32 Width = Event.window.data1;
                i32 Height = Event.window.data2;
                glViewport(0, 0, Width, Height);
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
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
    
    // TODO(Jsanchez): Turn debug off on release
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    // TODO: Platform->CreateWindow("Untitled", 1366, 768);
    SDL_Window* Window = SDL_CreateWindow("Untitled", 1366, 768, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    SDL_GLContext GLContext = SDL_GL_CreateContext(Window);

    Renderer* Render = new Renderer(Window);
    Render->Init();

    // Camera Configuration
    // TODO: What type of projection is better for 2D?    
    glm::vec3 CameraPosition = glm::vec3(0.0f, 0.0f, 3.0f); 
    glm::vec3 CameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); 
    glm::vec3 CameraUp = glm::vec3(0.0f, 1.0f, 0.0f); 
    glm::mat4 View = glm::lookAt(CameraPosition, CameraTarget, CameraUp);
    glm::mat4 Projection = glm::ortho(-240.0f, 240.0f, -135.0f, 135.0f, 0.1f, 100.0f);
    
    font_data Roboto = LoadFontData("fonts/Arial.json", "fonts/Arial.png");
    
    while (IsRunning)
    {
        ProcessEvents();

        Render->BeginFrame();
        
        // Upload Camera Settings
        i32 ViewLocation = glGetUniformLocation(Render->ExampleShader, "View");
        i32 ProjectionLocation = glGetUniformLocation(Render->ExampleShader, "Projection");
        glUniformMatrix4fv(ViewLocation, 1, GL_FALSE, glm::value_ptr(View));
        glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, glm::value_ptr(Projection));

        // Update and set rendering variables to quad!
        glm::vec3 QuadScale = glm::vec3(80.0f, 80.0, 40.0f);
        glm::vec3 QuadPosition = {QuadPositionX, 0.0f, 0.0f};
        glm::mat4 Model = glm::mat4(1.0f);
        Model = glm::scale(Model, QuadScale);
        Model = glm::translate(Model, QuadPosition);
        i32 ModelLocation = glGetUniformLocation(Render->ExampleShader, "Model");
        glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, glm::value_ptr(Model));
        
        // Draw the quad
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        Render->EndFrame();
    }
    
    SDL_Quit();
}