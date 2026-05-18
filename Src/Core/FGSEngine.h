#pragma once

#include "DeletionStack.h"
#include "GPU/VkTypes.h"
#include "GPU/VulkanContext.h"
#include "GPU/VulkanQueue.h"
#include "Window.h"
#include "GPU/FrameData.h"
#include "GPU/VulkanDescriptors.h"
#include "GPU/VulkanSwapchain.h"
#include "GPU/VulkanImage.h"
#include "GPU/VulkanPipeline.h"


constexpr unsigned int FRAME_OVERLAP = 2;

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

    FrameData _frames[FRAME_OVERLAP];
    uint32_t _frameNumber = 0;

    VkCommandPool _immediateCommandPool;
    VkCommandBuffer _immediateCommandBuffer;
    VkFence _immediateFence;

    // The Draw Image is the image that fills the window so later on surely it will be a sourceImage and previewImage but this is a composition of that 2 and the editor UI
    // later on this image is copied to the swapchain image to be presented to the screen
    VulkanImage _drawImage; // Extent of this draw image is representing is original resolution not the window draw resolution
    Descriptors::DescriptorAllocator _drawImageAllocator;
    VkDescriptorSet _drawImageDescriptorSet;
    VkDescriptorSetLayout _drawImageDescriptorSetLayout;
    VkExtent2D _drawExtent; // Used in the actual draw

    // TODO: Change this to the graph system
    VulkanPipeline _drawPipeline;

    VmaAllocator _allocator;
    DeletionStack _mainDeletionStack;

private:
    inline FrameData& GetCurrentFrame() { return _frames[_frameNumber % FRAME_OVERLAP]; }
    
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
    void InitSyncStructures();
    void InitDescriptors(); // TODO: This will change a lot at least the implementation when the cache and gNode logic is added, right now just a simple binding 0 of a image nothing more
    void InitPipelines();
    void InitUI();

    void CreateSwapChain(uint32_t width, uint32_t height);


    void Draw();
};
