#include "TexturePool.h"

#include "Logger/Logger.h"
#include "Utils/AssetsUtils.h"
#include "Utils/FormatUtils.h"

TexturePool::TexturePool()
{
    _resourceManager = nullptr;
}

TexturePool::TexturePool(GPUResourceManager* resourceManager)
{
    _resourceManager = resourceManager;
}

GPUImageHandle TexturePool::CreateTexture(Graph::TextureDesc textureDesc)
{
    return _resourceManager->CreateImage(VkExtent3D(textureDesc.width, textureDesc.height, 1), FormatUtils::CastEImageFormatToVkFormat(textureDesc.format),
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
}

GPUImageHandle TexturePool::GetTexture(Graph::TextureDesc textureDesc)
{
    std::queue<GPUImageHandle>* pool = &_texturePools[textureDesc];

    if (pool->empty())
    {
        GPUImageHandle textureHandle = CreateTexture(textureDesc);
        
        _textureDescsByHandle[textureHandle] = textureDesc;
        
        _texturesInUse.insert(textureHandle);
        
        return textureHandle;
    }

    GPUImageHandle textureHandle = pool->front();
    pool->pop();
    
    _textureDescsByHandle[textureHandle] = textureDesc;

    auto success = _texturesInUse.insert(textureHandle);
    if (!success.second)
    {
        Logger::Log(Logger::LogLevel::Error, "TexturePool::GetTexture: Texture is already in use");
        return GPUImageHandle(0);
    }
    
    return textureHandle;
}

void TexturePool::ReleaseTexture(GPUImageHandle textureHandle)
{
    if (!_texturesInUse.contains(textureHandle))
    {
        Logger::Log(Logger::LogLevel::Warning, "TexturePool::ReleaseTexture: Texture is not in use");
        return;
    }

    _texturesInUse.erase(textureHandle);

   auto textureDescRef = _textureDescsByHandle.find(textureHandle);
    if (textureDescRef == _textureDescsByHandle.end())
    {
        Logger::Log(Logger::LogLevel::Warning, "TexturePool::ReleaseTexture: TextureDesc is not found");
        return;
    }
    
    std::queue<GPUImageHandle>* pool = &_texturePools[textureDescRef->second];

    pool->push(textureHandle);
}

void TexturePool::ClearPools()
{
    for (auto[format, pool] : _texturePools)
    {
        size_t s = pool.size();
        for (size_t i = 0; i < s; ++i)
        {
            GPUImageHandle textureHandle = pool.front();
            pool.pop();

            if (AssetsUtils::IsValid(textureHandle))
            {
                _resourceManager->DestroyImage(textureHandle);
            }
        }
    }

    for (auto texture : _texturesInUse)
    {
        if (AssetsUtils::IsValid(texture))
        {
            _resourceManager->DestroyImage(texture);
        }
    }
    _texturesInUse.clear();
}
