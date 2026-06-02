#pragma once

#include <queue>

#include "GPUTypes.h"


namespace TextureLoader
{
    struct TextureResult;
}

class Renderer;


using GPUImageHandle = uint32_t;
using GPUBufferHandle = uint32_t;

// This manager is for the resources that will live between frames like the uniform buffers and images that will be uploaded to a pass that a node has
// Note that if an image or buffer is Created with this manager it needs to be free also with this manager Destroy functions (for clean the resource map)
// Recommended to store just the handle and when you will use the resource use the getter (The pointer maybe be invalid if other resources are added or removed from the manager)
// The Transient images will be done with a pool of images per frame after compute how many of them are needed to the graph execution
class GPUResourceManager
{
public:
    GPUResourceManager() = default;
    GPUResourceManager(Renderer* renderer);
    ~GPUResourceManager();

    GPUImageHandle CreateAndFillImage(TextureLoader::TextureResult* textureData, VkImageUsageFlags usageFlags);
    GPUImageHandle CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usageFlags);
    VulkanImage* GetImage(GPUImageHandle imageHandle);
    void DestroyImage(GPUImageHandle imageHandle);
    
    GPUBufferHandle CreateBuffer(size_t allocSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage);
    VulkanBuffer* GetBuffer(GPUImageHandle bufferHandle);
    void DestroyBuffer(GPUImageHandle bufferHandle);
    
    void ClearAll();
    void ClearImages();
    void ClearBuffers();

private:
    Renderer* _renderer;

    GPUImageHandle _imageNextID = 1;

    GPUBufferHandle _bufferNextID = 1;

    std::unordered_map<GPUImageHandle, VulkanImage> _images;
    std::queue<GPUImageHandle> _imagesFreeIDs;

    std::unordered_map<GPUBufferHandle, VulkanBuffer> _buffers;
    std::queue<GPUBufferHandle> _buffersFreeIDs;
};
