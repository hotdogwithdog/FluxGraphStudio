#pragma once
#include "NodesResourceManager.h"
#include "TexturePool.h"


class RenderCommand;
class GPUResourceManager;

class RenderGraph
{
public:

public:
    RenderGraph();
    RenderGraph(GPUResourceManager* resourceManager);
    ~RenderGraph() = default;

    void LoadCompiledGraph(std::vector<DescriptionNodeInstanceHandle>& nodes);

    void Render(VkCommandBuffer& cmd, const VulkanContext& context, VmaAllocator& allocator, const VulkanImage& sourceImage, const VulkanImage& previewImage);
    
private:
    GPUResourceManager* _resourceManager;

    TexturePool _texturePool;

    std::vector<RenderCommand*> _renderCommands;
};
