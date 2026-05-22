#pragma once

#include "GPU/VkTypes.h"
#include "GPU/GPUTypes.h"


namespace TextureLoader
{
    struct TextureResult;
}

class Window;

static constexpr unsigned int FRAME_OVERLAP = 2;


class Renderer
{
public:


public:
    Renderer() = default;
    
    void Init(Window* window);

    void Render();

    void CleanUp();

    
private:

    bool _bIsInitialized = false;
    
#ifdef _DEBUG
    const bool _bValidationLayers = true;
#else
    const bool _bValidationLayers = false;
#endif

    Window* _window;
    
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

    // Default samplers
    VkSampler _defaultSamplerNearest;
    VkSampler _defaultSamplerLinear;

    // TODO: Change this to the graph system
    VulkanImage _testImage;
    VkDescriptorSetLayout _commonDescriptorSetLayout;

    // TODO: Change this to the graph system
    VulkanPipeline _drawPipeline;

    VmaAllocator _allocator;
    DeletionStack _mainDeletionStack;
    
private:
    void InitVulkan();
    void InitSwapChain();
    void InitCommands();
    void InitSyncStructures();
    void InitDescriptors(); // TODO: This will change a lot at least the implementation when the cache and gNode logic is added, right now just a simple binding 0 of a image nothing more
    void InitPipelines();
    void InitDefaultSamplers();
    void InitTestImage(); // TODO: This will be removed when the graph logic is running just for test the upload of images

    void DestroySwapchain();
    void ResizeSwapchain();

    void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
    VulkanImage CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usageFlags);
    VulkanImage CreateAndFillImage(TextureLoader::TextureResult* textureData, VkImageUsageFlags usageFlags);
    
    VulkanBuffer CreateBuffer(size_t allocSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage);
    void DestroyBuffer(const VulkanBuffer& buffer);


    void Draw();

    
    inline FrameData& GetCurrentFrame() { return _frames[_frameNumber % FRAME_OVERLAP]; }
};
