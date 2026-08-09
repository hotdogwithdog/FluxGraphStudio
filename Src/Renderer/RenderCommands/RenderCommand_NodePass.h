#pragma once

#include "RenderCommand.h"
#include "Assets/AssetsManager.h"

class RenderCommand_NodePass : public RenderCommand
{
protected:
    NodeInfoHandle nodeInfoHandle;
    
public:
    RenderCommand_NodePass();

    RenderCommand_NodePass(const NodeInfoHandle& inNodeInfoHandle);

    inline void SetNodeInfoHandle(const NodeInfoHandle& inNodeInfoHandle) { nodeInfoHandle = inNodeInfoHandle; }
    
    virtual void Execute(const VkCommandBuffer& cmd) const override;
};
