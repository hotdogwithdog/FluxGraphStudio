#pragma once

#include "Window.h"
#include "Editor/Editor.h"
#include "Renderer/Renderer.h"
#include "SDL3/SDL_events.h"


class FGSEngine
{
public:
    FGSEngine() = default;

    static FGSEngine* GetInstance();

    void Init();

    void Run();

    void CleanUp();

private:
    bool _bIsInitialized = false;

    Window _window = Window(1700, 900, "Flux Graph Studio");

    Renderer _renderer;

    Editor _editor;

private:
    bool PollEvents(SDL_Event& e);
};
