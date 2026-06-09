#pragma once
#include "NodesResourceManager.h"
#include "TexturePool.h"


class GPUResourceManager;

class RenderGraph
{
public:

public:
    RenderGraph();
    RenderGraph(GPUResourceManager* resourceManager);
    ~RenderGraph() = default;
    
private:
    GPUResourceManager* _resourceManager;

    TexturePool _texturePool;

    
    
};
