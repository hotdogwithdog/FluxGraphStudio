#include "RGNode.h"

RGNode::RGNode()
{
    this->nodeInfoHandle = 0;
    this->version = 0;
    this->pipeline.pipeline = VK_NULL_HANDLE;
    this->pipeline.layout = VK_NULL_HANDLE;
    this->descriptorSetLayout = VK_NULL_HANDLE;
}

RGNode::RGNode(NodeInfoHandle nodeInfoHandle, uint32_t version)
{
    this->nodeInfoHandle = nodeInfoHandle;
    this->version = version;
    this->pipeline.pipeline = VK_NULL_HANDLE;
    this->pipeline.layout = VK_NULL_HANDLE;
    this->descriptorSetLayout = VK_NULL_HANDLE;
}
