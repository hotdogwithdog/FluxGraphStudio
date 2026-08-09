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
    TexturePool();
    TexturePool(GPUResourceManager* resourceManager);
    ~TexturePool() = default;
    
    GPUImageHandle GetTexture(Graph::TextureDesc textureDesc);
    void ReleaseTexture(GPUImageHandle textureHandle);

    void ClearPools();

private:
    GPUImageHandle CreateTexture(Graph::TextureDesc textureDesc);

private:
    GPUResourceManager* _resourceManager;
    
    std::unordered_map<Graph::TextureDesc, std::queue<GPUImageHandle>> _texturePools;

    std::unordered_map<GPUImageHandle, Graph::TextureDesc> _textureDescsByHandle;

    std::unordered_set<GPUImageHandle> _texturesInUse; 
};
