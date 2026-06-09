#pragma once

#include "Assets/AssetsManager.h"
#include "GPU/GPUTypes.h"

// This class is the Node pass class itself, so it has the logic for his execution
class RGNode
{
private:
    // Node for conections
    NodeInfoHandle _nodeInfoHandle;
    
    VulkanPipeline _pipeline;
    VkDescriptorSetLayout _descriptorSetLayout;
    
public:
    RGNode(NodeInfoHandle nodeInfoHandle);
    
};
