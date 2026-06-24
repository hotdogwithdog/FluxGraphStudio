#pragma once

#include <imgui.h>

#include "Editor/EditorContext.h"
#include "GPU/GPUResourceManager.h"
#include "GPU/VkTypes.h"
#include "GPU/GPUTypes.h"
#include "Graph/RenderGraph.h"


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

    void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
    VulkanImage CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usageFlags);
    VulkanImage CreateAndFillImage(TextureLoader::TextureResult* textureData, VkImageUsageFlags usageFlags);
    void DestroyImage(VulkanImage image);

    
    VulkanBuffer CreateBuffer(size_t allocSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage);
    void DestroyBuffer(const VulkanBuffer& buffer);

    
private:

    bool _bIsInitialized = false;
    
#ifdef _DEBUG
    const bool _bValidationLayers = true;
#else
    const bool _bValidationLayers = false;
#endif

    Window* _window;

    GPUResourceManager _gpuResourceManager = GPUResourceManager(this);
    RenderGraph _renderGraph;
    
    VulkanContext _vulkanContext;
    VulkanQueue _mainQueue;
    VulkanQueue _immediateQueue;

    VulkanSwapchain _swapchain;

    FrameData _frames[FRAME_OVERLAP];
    uint32_t _frameNumber = 0;

    VkCommandPool _immediateCommandPool;
    VkCommandBuffer _immediateCommandBuffer;
    VkFence _immediateFence;

    Descriptors::DescriptorAllocator _drawImageAllocator;
    VkDescriptorSet _drawImageDescriptorSet;
    VkDescriptorSetLayout _drawImageDescriptorSetLayout;

    // Default samplers
    VkSampler _defaultSamplerNearest;
    VkSampler _defaultSamplerLinear;

    // TODO: Change this to the graph system
    VkDescriptorSetLayout _commonDescriptorSetLayout;

    // TODO: Change this to the graph system
    VulkanPipeline _drawPipeline;

    VmaAllocator _allocator;
    DeletionStack _mainDeletionStack;


    // TODO: Remove this
    EditorImage _sourceImage;
    EditorImage _previewImage;
    
private:
    void InitVulkan();
    void InitSwapChain();
    void InitPreviewImage();
    void InitCommands();
    void InitSyncStructures();
    void InitDescriptors(); // TODO: This will change a lot at least the implementation when the cache and gNode logic is added, right now just a simple binding 0 of a image nothing more
    void InitPipelines();
    void InitDefaultSamplers();
    void InitSourceImage(); // TODO: This will be removed when the graph logic is running just for test the upload of images
    void InitImGui();

    void DestroySwapchain();
    void ResizeSwapchain();
    

    void Draw();

    void DrawEditor(VkCommandBuffer cmd, VkImageView targetImageView);

    
    inline FrameData& GetCurrentFrame() { return _frames[_frameNumber % FRAME_OVERLAP]; }
};
