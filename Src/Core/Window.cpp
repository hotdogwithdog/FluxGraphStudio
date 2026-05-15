#include "Window.h"

#include <SDL3/SDL.h>


void Window::Init()
{
    SDL_WindowFlags windowFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    window = SDL_CreateWindow(_title.c_str(),
        windowExtent.width,
        windowExtent.height,
        windowFlags);
}


