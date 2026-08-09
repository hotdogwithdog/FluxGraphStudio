#pragma once

#include "NodesResourceManager.h"
#include "Assets/AssetsManager.h"
#include "Graph/RGNode.h"
#include "GPU/GPUTypes.h"


class NodeLibrary
{
public:
    NodeLibrary() = default;
    NodeLibrary(VulkanContext* vulkanContext, VkDescriptorSetLayout commonDescriptorSetLayout);

    RGNode* GetNodeFromDescription(DescriptionNodeInstanceHandle handle);
    RGNode* GetNodeFromInfo(NodeInfoHandle handle);
    
    
private:
    VulkanContext* _vulkanContext;

    VkDescriptorSetLayout _commonDescriptorSetLayout;

    std::unordered_map<NodeInfoHandle, RGNode> _nodes;

private:
    void InitializeNode(NodeInfoHandle handle);
};
