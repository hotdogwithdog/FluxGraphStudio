#pragma once
#include "NodeLibrary.h"
#include "NodesResourceManager.h"
#include "TexturePool.h"


class RenderCommand;
class GPUResourceManager;

class RenderGraph
{
public:

public:
    RenderGraph();
    RenderGraph(GPUResourceManager* resourceManager, VulkanContext* vulkanContext, VkDescriptorSetLayout commonDescriptorSetLayout);
    ~RenderGraph() = default;

    void LoadCompiledGraph(std::vector<DescriptionNodeInstanceHandle>& nodes);

    void Render(const VkCommandBuffer& cmd, const VulkanContext& context, VmaAllocator& allocator, const VulkanImage& sourceImage, const VulkanImage& previewImage);

    void Test(NodeInfoHandle handle); 
    
private:
    GPUResourceManager* _resourceManager;
    TexturePool _texturePool;
    NodeLibrary _nodeLibrary;

    std::vector<RenderCommand*> _renderCommands;
};
