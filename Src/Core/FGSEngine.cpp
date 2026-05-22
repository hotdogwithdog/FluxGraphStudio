#include "FGSEngine.h"

#include <cassert>
#include <chrono>
#include <thread>

#include "VkBootstrap.h"


FGSEngine* loadedEngine = nullptr;
FGSEngine* FGSEngine::GetInstance() { return loadedEngine; }


void FGSEngine::Init()
{
    assert(loadedEngine == nullptr);
    loadedEngine = this;

    _renderer.Init(&_window);

    // TODO: Editor Init
    

    _bIsInitialized = true;
}

void FGSEngine::Run()
{
    SDL_Event e;
    bool bQuit = false;
    
    while (!bQuit)
    {
        bQuit = PollEvents(e);

        if (_window.isMinimized)
        {
            // Sleep for a little to not overload the CPU while the app si minimized // TODO: Probably can be done in a really better way
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        _renderer.Render();
        
        // TODO: Editor
    }
}

void FGSEngine::CleanUp()
{
    if (!_bIsInitialized) return;

    _renderer.CleanUp();
    
}

bool FGSEngine::PollEvents(SDL_Event& e)
{
    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_EVENT_QUIT)
        {
            return true;
        }

        if (e.type == SDL_EVENT_WINDOW_MINIMIZED)
        {
            _window.isMinimized = true;
        }
        if (e.type == SDL_EVENT_WINDOW_RESTORED)
        {
            _window.isMinimized = false;
        }
    }

    return false;
}
