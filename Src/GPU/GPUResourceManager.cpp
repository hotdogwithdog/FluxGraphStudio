#include "GPUResourceManager.h"

#include "Utils/AssetsUtils.h"
#include "Renderer/Renderer.h"

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

    GPUImageHandle imageHandle;
    if (!_imagesFreeIDs.empty())
    {
        imageHandle = _imagesFreeIDs.front();
        _imagesFreeIDs.pop();
    }
    else
    {
        imageHandle = _imageNextID;
        _imageNextID++;
    }
    
    auto succed = _images.insert(std::make_pair(imageHandle, _renderer->CreateAndFillImage(textureData, usageFlags)));
    if (!succed.second)
    {
        // The code Never should reach here because i just increment the _nextID so if it fails the ID is not unique
        Logger::Log(Logger::LogLevel::Error, "GPUResourceManager::CreateAndFillImage: Failed to Insert GPU image, so the image already exist!");
        return 0;
    }
    
    return imageHandle;
}

GPUImageHandle GPUResourceManager::CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usageFlags)
{
    GPUImageHandle imageHandle;
    if (!_imagesFreeIDs.empty())
    {
        imageHandle = _imagesFreeIDs.front();
        _imagesFreeIDs.pop();
    }
    else
    {
        imageHandle = _imageNextID;
        _imageNextID++;
    }
    
    auto succed = _images.insert(std::make_pair(_imageNextID, _renderer->CreateImage(size, format, usageFlags)));
    if (!succed.second)
    {
        // The code Never should reach here because i just increment the _nextID so if it fails the ID is not unique
        Logger::Log(Logger::LogLevel::Error, "GPUResourceManager::CreateImage: Failed to Insert GPU image, so the image already exist");
        return 0;
    }
    
    return imageHandle;
}

VulkanImage* GPUResourceManager::GetImage(GPUImageHandle imageHandle)
{
    if (!AssetsUtils::IsValid(imageHandle))
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
    GPUBufferHandle bufferHandle;
    if (!_imagesFreeIDs.empty())
    {
        bufferHandle = _buffersFreeIDs.front();
        _buffersFreeIDs.pop();
    }
    else
    {
        bufferHandle = _bufferNextID;
        _bufferNextID++;
    }
    
    auto succed = _buffers.insert(std::make_pair(bufferHandle, _renderer->CreateBuffer(allocSize, usageFlags, memoryUsage)));
    if (!succed.second)
    {
        Logger::Log(Logger::LogLevel::Error, "GPUResourceManager::CreateBuffer: Failed to Insert GPU Buffer, so the buffer already exist");
        return 0;
    }
    
    return bufferHandle;
}

VulkanBuffer* GPUResourceManager::GetBuffer(GPUImageHandle bufferHandle)
{
    if (!AssetsUtils::IsValid(bufferHandle))
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
    if (!AssetsUtils::IsValid(bufferHandle))
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
    _buffersFreeIDs.push(bufferHandle);
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
    const size_t s = _imagesFreeIDs.size();
    for (size_t i = 0; i < s; i++)
    {
        _imagesFreeIDs.pop();
    }
    _imageNextID = 1;
}

void GPUResourceManager::ClearBuffers()
{
    for (auto [key, value] : _buffers)
    {
        _renderer->DestroyBuffer(value);
    }
    _buffers.clear();
    const size_t s = _buffersFreeIDs.size();
    for (size_t i = 0; i < s; i++)
    {
        _buffersFreeIDs.pop();
    }
    _bufferNextID = 1;
}

void GPUResourceManager::DestroyImage(GPUImageHandle imageHandle)
{
    if (!AssetsUtils::IsValid(imageHandle))
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
    _imagesFreeIDs.push(imageHandle);
}
