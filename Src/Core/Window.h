#pragma once

#include <string>

#include "GPU/VkTypes.h"
#include "SDL3/SDL_video.h"

class Window
{
private:
    std::string _title;

    bool _bResizeRequested = false;
    
public:
    struct SDL_Window* window{nullptr};
    VkExtent2D windowExtent;

    bool isMinimized = false;
    
public:
    Window(uint32_t width, uint32_t height, const std::string& title) : _title(title), windowExtent(width, height) { }

    void Init();
};
