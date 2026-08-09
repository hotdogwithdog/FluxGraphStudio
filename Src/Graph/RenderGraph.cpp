#include "RenderGraph.h"

#include "GPU/GPUResourceManager.h"

RenderGraph::RenderGraph()
{
    _resourceManager = nullptr;
}

RenderGraph::RenderGraph(GPUResourceManager* resourceManager, VulkanContext* vulkanContext, VkDescriptorSetLayout commonDescriptorSetLayout)
{
    _resourceManager = resourceManager;
    _texturePool = TexturePool(_resourceManager);
    _nodeLibrary = NodeLibrary(vulkanContext, commonDescriptorSetLayout);
}

void RenderGraph::LoadCompiledGraph(std::vector<DescriptionNodeInstanceHandle>& nodes)
{
    
}

void RenderGraph::Render(const VkCommandBuffer& cmd, const VulkanContext& context, VmaAllocator& allocator, const VulkanImage& sourceImage, const VulkanImage& previewImage)
{
    
}

void RenderGraph::Test(NodeInfoHandle handle)
{
    RGNode* node = _nodeLibrary.GetNodeFromInfo(handle);
    if (node == nullptr) return;
}
