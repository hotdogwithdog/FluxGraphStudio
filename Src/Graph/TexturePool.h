#pragma once

#include <unordered_set>

#include "RGNodeInfo.h"
#include "GPU/GPUResourceManager.h"

class GPUResourceManager;


/// This pool is just used for the transient textures of the graph,
/// it uses handles that are fixed to the GPUResourceManager that uses the renderer in which the RenderGraph is created
/// so it is just a wrapper to that GPUResourceManager for the use of the Graph
class TexturePool
{
public:
    struct PooleableTexture
    {
    private:
        GPUImageHandle _textureHandle;

        friend class TexturePool;
    public:
        VulkanImage* texture;
        
        PooleableTexture(GPUImageHandle textureHandle)
        {
            _textureHandle = textureHandle;
        }
    };

    
public:
    TexturePool();
    TexturePool(GPUResourceManager* resourceManager);
    ~TexturePool() = default;

    // The size will be the inputImageSize for all of them and the usageFlags will be the same for all of them
    void CreateTexture(VkExtent3D size, Graph::EImageFormat format, VkImageUsageFlags usageFlags);

    PooleableTexture GetTexture(Graph::EImageFormat format);
    void ReleaseTexture(PooleableTexture texture);

    void ClearPools();

private:
    GPUResourceManager* _resourceManager;
    
    std::unordered_map<Graph::EImageFormat, std::queue<PooleableTexture>> _texturePools;

    std::unordered_set<GPUImageHandle> _texturesInUse; 
};
