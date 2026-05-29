#include "GPUResourceManager.h"

#include "Renderer/Renderer.h"

bool IsValid(uint32_t resourceHandle)
{
    return resourceHandle != 0;
}

GPUResourceManager::GPUResourceManager(Renderer* renderer)
{
    _renderer = renderer;
}

GPUResourceManager::~GPUResourceManager()
{
    ClearAll();
}

GPUImageHandle GPUResourceManager::CreateAndFillImage(TextureLoader::TextureResult* textureData, VkImageUsageFlags usageFlags)
{
    auto succed = _images.insert(std::make_pair(_imageNextID, _renderer->CreateAndFillImage(textureData, usageFlags)));
    if (!succed.second)
    {
        // The code Never should reach here because i just increment the _nextID so if it fails the ID is not unique
        Logger::Log(Logger::LogLevel::Error, "GPUResourceManager::CreateAndFillImage: Failed to Insert GPU image, so the image already exist!");
        _imageNextID++;
        return 0;
    }
    
    GPUImageHandle handle = _imageNextID;
    _imageNextID++;
    return handle;
}

GPUImageHandle GPUResourceManager::CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usageFlags)
{
    auto succed = _images.insert(std::make_pair(_imageNextID, _renderer->CreateImage(size, format, usageFlags)));
    if (!succed.second)
    {
        // The code Never should reach here because i just increment the _nextID so if it fails the ID is not unique
        Logger::Log(Logger::LogLevel::Error, "GPUResourceManager::CreateImage: Failed to Insert GPU image, so the image already exist");
        _imageNextID++;
        return 0;
    }
    
    GPUImageHandle handle = _imageNextID;
    _imageNextID++;
    return handle;
}

VulkanImage* GPUResourceManager::GetImage(GPUImageHandle imageHandle)
{
    if (!IsValid(imageHandle))
    {
        Logger::Log(Logger::LogLevel::Error, "GPUResourceManager::GetImage: Image handle is invalid");
        return nullptr;
    }

    if (!_images.contains(imageHandle))
    {
        Logger::Log(Logger::LogLevel::Error, std::format("GPUResourceManager::GetImage: Image with handle = {}; not found", imageHandle));
        return nullptr;
    }

    return &_images[imageHandle];
}

GPUBufferHandle GPUResourceManager::CreateBuffer(size_t allocSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage)
{
    auto succed = _buffers.insert(std::make_pair(_bufferNextID, _renderer->CreateBuffer(allocSize, usageFlags, memoryUsage)));
    if (!succed.second)
    {
        Logger::Log(Logger::LogLevel::Error, "GPUResourceManager::CreateBuffer: Failed to Insert GPU Buffer, so the buffer already exist");
        _bufferNextID++;
        return 0;
    }

    GPUBufferHandle handle = _bufferNextID;
    _bufferNextID++;
    return handle;
}

VulkanBuffer* GPUResourceManager::GetBuffer(GPUImageHandle bufferHandle)
{
    if (!IsValid(bufferHandle))
    {
        Logger::Log(Logger::LogLevel::Error, "GPUResourceManager::GetBuffer: Buffer handle is invalid");
        return nullptr;
    }

    if (!_buffers.contains(bufferHandle))
    {
        Logger::Log(Logger::LogLevel::Error, std::format("GPUResourceManager::GetBuffer: Buffer with handle = {}; not found", bufferHandle));
        return nullptr;
    }

    return &_buffers[bufferHandle];
}

void GPUResourceManager::DestroyBuffer(GPUImageHandle bufferHandle)
{
    if (!IsValid(bufferHandle))
    {
        Logger::Log(Logger::LogLevel::Warning, "GPUResourceManager::DestroyBuffer: Buffer Handle Not Valid");
        return;
    }
    
    if (!_buffers.contains(bufferHandle))
    {
        Logger::Log(Logger::LogLevel::Warning, std::format("GPUResourceManager::DestroyBuffer: Key not found: {}", bufferHandle));
        return;
    }

    VulkanBuffer buffer = _buffers[bufferHandle];
    _renderer->DestroyBuffer(buffer);
    
    _buffers.erase(bufferHandle);
}

void GPUResourceManager::ClearAll()
{
    ClearImages();
    ClearBuffers();
}

void GPUResourceManager::ClearImages()
{
    for (auto [key, value] : _images)
    {
        _renderer->DestroyImage(value);
    }
    _images.clear();
    _imageNextID = 1;
}

void GPUResourceManager::ClearBuffers()
{
    for (auto [key, value] : _buffers)
    {
        _renderer->DestroyBuffer(value);
    }
    _buffers.clear();
    _bufferNextID = 1;
}

void GPUResourceManager::DestroyImage(GPUImageHandle imageHandle)
{
    if (!IsValid(imageHandle))
    {
        Logger::Log(Logger::LogLevel::Warning, "GPUResourceManager::DestroyImage: Imager Handle Not Valid");
        return;
    }
    
    if (!_images.contains(imageHandle))
    {
        Logger::Log(Logger::LogLevel::Warning, std::format("GPUResourceManager::DestroyImage: Key not found: {}", imageHandle));
        return;
    }

    VulkanImage image = _images[imageHandle];
    _renderer->DestroyImage(image);
    
    _images.erase(imageHandle);
}
