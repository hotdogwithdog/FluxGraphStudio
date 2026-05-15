#pragma once

#include "DeletionStack.h"
#include "GPU/VkTypes.h"
#include "GPU/VulkanContext.h"
#include "GPU/VulkanQueue.h"
#include "Window.h"
#include "GPU/VulkanSwapchain.h"
#include "GPU/VulkanImage.h"


class FGSEngine
{
private:

    bool _bIsInitialized = false;
    
#ifdef _DEBUG
    const bool _bValidationLayers = true;
#else
    const bool _bValidationLayers = false;
#endif

    Window _window = Window(1700, 900, "Flux Graph Studio");

    VulkanContext _vulkanContext;
    VulkanQueue _mainQueue;
    VulkanQueue _immediateQueue;

    VulkanSwapchain _swapchain;

    // The Draw Image is the image that fills the window so later on surely it will be a sourceImage and previewImage but this is a composition of that 2 and the editor UI
    // later on this image is copied to the swapchain image to be presented to the screen
    VulkanImage _drawImage;

    VmaAllocator _allocator;
    DeletionStack _mainDeletionStack;
    
public:
    FGSEngine() = default;

    static FGSEngine* GetInstance();

    void Init();

    void Run();

    void CleanUp();

private:
    void InitVulkan();
    void InitSwapChain();
    void InitCommands();
    void InitUI();

    void CreateSwapChain(uint32_t width, uint32_t height);


    void Draw();
};
