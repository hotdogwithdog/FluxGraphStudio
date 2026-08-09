#include "RenderCommand_NodePass.h"

#include "Assets/AssetsManager.h"
#include "Graph/RGNodeInstance.h"

RenderCommand_NodePass::RenderCommand_NodePass()
{
    nodeInfoHandle = 0; // The NULL Value for handles
}

RenderCommand_NodePass::RenderCommand_NodePass(const NodeInfoHandle& inNodeInstanceHandle)
{
    nodeInfoHandle = inNodeInstanceHandle;
}

void RenderCommand_NodePass::Execute(const VkCommandBuffer& cmd) const
{
    
}
