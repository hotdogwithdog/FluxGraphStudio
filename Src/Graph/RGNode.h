#pragma once

#include "Assets/AssetsManager.h"
#include "GPU/GPUTypes.h"

// This class is the Node pass class itself, so it has the logic for his execution
class RGNode
{
public:
    NodeInfoHandle nodeInfoHandle;
    uint32_t version = 1;
    
    VulkanPipeline pipeline;
    VkDescriptorSetLayout descriptorSetLayout;
    
public:
    RGNode();
    RGNode(NodeInfoHandle nodeInfoHandle, uint32_t version);
};
