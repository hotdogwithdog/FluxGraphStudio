#include "TexturePool.h"


#include "Logger/Logger.h"
#include "Utils/AssetsUtils.h"
#include "Utils/FormatUtils.h"

TexturePool::TexturePool()
{
    _resourceManager = nullptr;
    Logger::Log(Logger::LogLevel::Error, "Creating Texture Pool without GPUResourceManager reference");
}

TexturePool::TexturePool(GPUResourceManager* resourceManager)
{
    _resourceManager = resourceManager;
}

void TexturePool::CreateTexture(VkExtent3D size, Graph::EImageFormat format, VkImageUsageFlags usageFlags)
{
    GPUImageHandle handle = _resourceManager->CreateImage(size, FormatUtils::CastEImageFormatToVkFormat(format), usageFlags);

    std::queue<PooleableTexture>* pool = &_texturePools[format];

    pool->emplace(handle);
}

TexturePool::PooleableTexture TexturePool::GetTexture(Graph::EImageFormat format)
{
    std::queue<PooleableTexture>* pool = &_texturePools[format];

    if (pool->empty())
    {
        Logger::Log(Logger::LogLevel::Error, "TexturePool::GetTexture: No textures of this format ready");
        return PooleableTexture(0);
    }

    PooleableTexture texture = pool->front();
    pool->pop();

    texture.texture = _resourceManager->GetImage(texture._textureHandle);

    auto success = _texturesInUse.insert(texture._textureHandle);
    if (!success.second)
    {
        Logger::Log(Logger::LogLevel::Error, "TexturePool::GetTexture: Texture is already in use");
        return PooleableTexture(0);
    }
    
    return texture;
}

void TexturePool::ReleaseTexture(PooleableTexture texture)
{
    if (!_texturesInUse.contains(texture._textureHandle))
    {
        Logger::Log(Logger::LogLevel::Warning, "TexturePool::ReleaseTexture: Texture is not in use");
        return;
    }

    _texturesInUse.erase(texture._textureHandle);

    // I do not want to use the texture pointer inside the texture because in this point it can be invalid so i just get it again
    Graph::EImageFormat format = FormatUtils::CastVkFormatToEImageFormat(_resourceManager->GetImage(texture._textureHandle)->imageFormat);
    std::queue<PooleableTexture>* pool = &_texturePools[format];

    pool->push(texture);
}

void TexturePool::ClearPools()
{
    for (auto[format, pool] : _texturePools)
    {
        size_t s = pool.size();
        for (size_t i = 0; i < s; ++i)
        {
            PooleableTexture texture = pool.front();
            pool.pop();

            if (AssetsUtils::IsValid(texture._textureHandle))
            {
                _resourceManager->DestroyImage(texture._textureHandle);
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
