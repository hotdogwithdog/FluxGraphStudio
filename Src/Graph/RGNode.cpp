#include "RGNode.h"

RGNode::RGNode(NodeInfoHandle nodeInfoHandle)
{
    _nodeInfoHandle = nodeInfoHandle;
}

void RGNode::Execute(VulkanContext& ctx, VkCommandBuffer& cmd)
{
    
}
