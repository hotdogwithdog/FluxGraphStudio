#pragma once

#include "GPU/VkTypes.h"
#include "Window.h"
#include "Renderer/Renderer.h"
#include "SDL3/SDL_events.h"


namespace TextureLoader
{
    struct TextureResult;
}

class FGSEngine
{
private:
    bool _bIsInitialized = false;

    Window _window = Window(1700, 900, "Flux Graph Studio");

    Renderer _renderer;

    
public:
    FGSEngine() = default;

    static FGSEngine* GetInstance();

    void Init();

    void Run();

    void CleanUp();
    

private:
    bool PollEvents(SDL_Event& e);
};
