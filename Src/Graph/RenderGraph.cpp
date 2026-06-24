#include "RenderGraph.h"

#include "GPU/GPUResourceManager.h"

RenderGraph::RenderGraph()
{
    Logger::Log(Logger::LogLevel::Error, "Render Graph created without resourceManager");
    _resourceManager = nullptr;
}

RenderGraph::RenderGraph(GPUResourceManager* resourceManager, VulkanContext* vulkanContext)
{
    _resourceManager = resourceManager;
    _texturePool = TexturePool(_resourceManager);
    _nodeLibrary = NodeLibrary(vulkanContext);
}

void RenderGraph::LoadCompiledGraph(std::vector<DescriptionNodeInstanceHandle>& nodes)
{
    
}

void RenderGraph::Render(VkCommandBuffer& cmd, const VulkanContext& context, VmaAllocator& allocator, const VulkanImage& sourceImage, const VulkanImage& previewImage)
{
    
}

void RenderGraph::Test(NodeInfoHandle handle)
{
    RGNode* node = _nodeLibrary.GetNodeFromInfo(handle);
    if (node == nullptr) return;
}
