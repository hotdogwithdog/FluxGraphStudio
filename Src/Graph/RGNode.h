#pragma once

#include "GPU/GPUTypes.h"

// This class is the Node pass class itself, so it has the logic for his execution
class RGNode
{
private:
    // Node for conections
    
    VulkanPipeline _pipeline;
    VkDescriptorSetLayout _descriptorSetLayout;
    
public:
    RGNode() = default;


    // Execute the Pass of the Node
    void Execute(VulkanContext& ctx, VkCommandBuffer& cmd);
    
};
